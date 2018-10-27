/*
 *  Copyright (C) 2014, 2015, 2016, 2017, 2018 Michael Hofmann
 *  Copyright (C) 2017 Thomas Weber
 *  
 *  This file is part of the Simulation Component and Data Coupling (SCDC) library.
 *  
 *  The SCDC library is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  The SCDC library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <cstdio>
#include <cstdarg>
#include <string>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

#include <nfsc/libnfs.h>
#include <nfsc/libnfs-raw-nfs.h>

#define SCDC_TRACE_NOT  !SCDC_TRACE_DATAPROV_NFS

#include "config.hh"
#include "common.hh"
#include "log.hh"
#include "dataprov_nfs2.hh"


using namespace std;


#define SCDC_LOG_PREFIX  "dataset-nfs-store: "


typedef struct _dataset_output_next_data_t
{
  struct nfs_context *nfs_ctx;
  struct nfsfh *nfs_file;

} dataset_output_next_data_t;


static scdcint_t dataset_output_next(scdc_dataset_output_t *output, scdc_result_t *result)
{
  dataset_output_next_data_t *data = static_cast<dataset_output_next_data_t *>(output->data);

  SCDC_TRACE_DATASET_OUTPUT(output, "dataset_output_next: ");

  SCDC_DATASET_INOUT_BUF_CURRENT(output) = 0;

  char *buf = static_cast<char *>(SCDC_DATASET_INOUT_BUF_PTR(output));
  scdcint_t buf_size = SCDC_DATASET_INOUT_BUF_SIZE(output);

  size_t n = 0;

  while (buf_size > 0)
  {
    SCDC_TRACE("dataset_output_next: buf_size: " << buf_size);

    n = nfs_read(data->nfs_ctx, data->nfs_file, buf_size, buf);

    if (n < 0)
    {
      /* FIXME */
      SCDC_FAIL("dataset_output_next: reading failed: " << nfs_get_error(data->nfs_ctx));
      break;
    }

    if (n == 0) break;

    buf += n;
    buf_size -= n;

    SCDC_DATASET_INOUT_BUF_CURRENT(output) += n;
  }

  if (n <= 0)
  {
    SCDC_TRACE("dataset_output_next: done");

    nfs_close(data->nfs_ctx, data->nfs_file);

    output->next = 0;
    output->data = 0;

    delete data;
  }

  return (n >= 0)?SCDC_SUCCESS:SCDC_FAILURE;
}


class scdc_dataset_nfs2_store: public scdc_dataset
{
  private:
    bool admin_mode;


    scdcint_t _count_fs_objects(struct nfsdir *dir, ftype3 fso_type)
    {
      scdc_dataprov_nfs2_store *dataprov_nfs = static_cast<scdc_dataprov_nfs2_store *>(dataprov);

      /* Read directory entries in as long as we have any */
      scdcint_t object_count = 0;
      struct nfsdirent *entry;
      while ((entry = nfs_readdir(dataprov_nfs->_nfs_context, dir)))
      {
        /* Skip current and parent directory */
        if (strcmp(entry->name, ".") == 0 || strcmp(entry->name, "..") == 0)
          continue;

        /* Count this entry if it's of the type we requested */
        if (entry->type == fso_type)
          ++object_count;
      }

      /* Rewind to the beginning to eliminate side effects */
      nfs_rewinddir(dataprov_nfs->_nfs_context, dir);

      return object_count;
    }


    bool _rm_recursive(const char *const path, scdc_dataset_output_t *output, scdc_result &result)
    {
      scdc_dataprov_nfs2_store *dataprov_nfs = static_cast<scdc_dataprov_nfs2_store *>(dataprov);

      /* Open the specified directory (because we need to delete the files
         in it first) */
      struct nfsdir *dir;
      if (nfs_opendir(dataprov_nfs->_nfs_context, path, &dir) != 0)
      {
        result = "entries of store cannot be enumerated";
        SCDC_FAIL("_rm_recursive: nfs_opendir('" << path << "') failed: " << dataprov_nfs->_libnfs_err());
        return false;
      }

      SCDC_TRACE("_rm_recursive: removing directory '" << path << "' including files");

      /* Read all entries in the directory. When hitting a directory, call us
         recursively. Otherwise, remove the file directly. */
      struct nfsdirent *entry;
      while ((entry = nfs_readdir(dataprov_nfs->_nfs_context, dir)))
      {
        /* Skip current and parent directory */
        if (strcmp(entry->name, ".") == 0 || strcmp(entry->name, "..") == 0)
          continue;

        /* Directory. Call us recursively to remove it. */
        if (entry->type == NF3DIR)
        {
          std::ostringstream recursive_path;
          recursive_path << path << "/" << entry->name;

          if (!_rm_recursive(recursive_path.str().c_str(), output, result))
          {
            nfs_closedir(dataprov_nfs->_nfs_context, dir);

            return false;
          }
        }

        /* Any other type (including regular files). Delete it directly. */
        else
        {
          std::ostringstream file_path;
          file_path << path << "/" << entry->name;
          std::string file_path_str = file_path.str();

          SCDC_TRACE("_rm_recursive: deleting file '" << file_path_str);

          /* Unlink the inode reference of the file, this is equivalent to
             deleting it from the specified path (but might be referenced in
             another path). */
          if (nfs_unlink(dataprov_nfs->_nfs_context, file_path_str.c_str()) != 0)
          {
            result = "cannot delete file '" + file_path_str + "'";
            SCDC_FAIL("_rm_recursive: nfs_unlink('" << file_path_str << "') failed: " << dataprov_nfs->_libnfs_err());
            nfs_closedir(dataprov_nfs->_nfs_context, dir);
            return false;
          }
        }
      }
      nfs_closedir(dataprov_nfs->_nfs_context, dir);

      /* The directory should be empty now; we can finally rmdir() it. */
      if (nfs_rmdir(dataprov_nfs->_nfs_context, path) != 0)
      {
        result = "empty directory cannot be deleted";
        SCDC_FAIL("_rm_recursive: nfs_rmdir('" << path << "') failed: " << dataprov_nfs->_libnfs_err());
        return false;
      }

      /* Finally gone. */
      SCDC_TRACE("_rm_recursive: removed directory '" << path << "'");
      return true;
    }


  public:
    scdc_dataset_nfs2_store(scdc_dataprov_nfs2_store *dataprov_)
      :scdc_dataset(dataprov_), admin_mode(false) { }


    bool do_cmd_info(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      /* Parameters are ignored. */
      SCDC_TRACE("do_cmd_info: '" << params << "'");

      scdc_dataprov_nfs2_store *dataprov_nfs = static_cast<scdc_dataprov_nfs2_store *>(dataprov);

      /*
       * Print the same information as the filesystem data provider:
       *
       *     "admin: (yes|no), stores: [0-9]+"
       *
       * The number of subdirectories in the share's root is equivalent to the
       * number of stores.
       */
      struct nfsdir *dir;
      int nfs_ret = nfs_opendir(dataprov_nfs->_nfs_context, "/", &dir);
      if (nfs_ret != 0)
      {
        result = "stores cannot be enumerated";
        SCDC_FAIL("do_cmd_info: nfs_opendir('/') failed: " << dataprov_nfs->_libnfs_err());
        return false;
      }

      scdcint_t stores = _count_fs_objects(dir, NF3DIR);

      nfs_closedir(dataprov_nfs->_nfs_context, dir);

      /* Print the line and succeed */
      char res[SCDC_RESULT_STR_MIN_SIZE];
      snprintf(res, SCDC_RESULT_STR_MIN_SIZE, "admin: %s, stores: %" scdcint_fmt, (admin_mode?"yes":"no"), stores);
      result = res;

      return true;
    }


    bool do_cmd_cd(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      /* The params string indicates the name of a store. "" is the current
         store. Any other store name can be considered as subdirectory, except
         for the special 'ADMIN' store, which allows to enumerate/create/delete
         subdirectories instead of files. */
      SCDC_TRACE("do_cmd_cd: '" << params << "'");

      /* Switch to admin mode. */
      if (params == "ADMIN")
        admin_mode = true;

      /* Keep or switch to regular mode. */
      else
        admin_mode = false;

      /*
       * Update the working directory within the dataset structure. We do not
       * use the nfs_chdir() function because SCDC does not allow tree
       * hierarchies, only a central store (backed by the NFS share) containing
       * only subdirectories of substores, and those in turn only files.
       * Therefore, we ensure that nfs_* calls are performed with absolute
       * paths to ensure the integrity of the hierarchy.
       *
       * scdc_dataset::do_cmd_cd() clears the output for us.
       */
      bool ret = scdc_dataset::do_cmd_cd(params, input, output, result);

      SCDC_TRACE("do_cmd_cd: return: " << ret);

      return ret;
    }


    bool do_cmd_ls(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      /* Parameters are ignored. */
      SCDC_TRACE("do_cmd_ls: '" << params << "'");

      scdc_dataprov_nfs2_store *dataprov_nfs = static_cast<scdc_dataprov_nfs2_store *>(dataprov);

      /*
       * Print subdirectories (if in admin mode or the current working
       * directory is not in a store). The output has the following format:
       *
       *  <number of directories>|<directory name 1>|<directory name 2>|...|
       */
      struct nfsdir *dir;
      struct nfsdirent *entry;
      if (admin_mode || pwd.size() == 0)
      {
        SCDC_TRACE("do_cmd_ls: listing stores");

        /* Open the share's root, containing the stores */
        if (nfs_opendir(dataprov_nfs->_nfs_context, "/", &dir) != 0)
        {
          result = "stores cannot be enumerated";
          SCDC_FAIL("do_cmd_ls: nfs_opendir('/') failed: " << dataprov_nfs->_libnfs_err());
          return false;
        }

        /* Count subdirectories first and print the number */
        scdcint_t stores = _count_fs_objects(dir, NF3DIR);
        SCDC_DATASET_OUTPUT_PRINTF_(output, "%" scdcint_fmt "|", stores);

        /* Now print the names */
        while ((entry = nfs_readdir(dataprov_nfs->_nfs_context, dir)))
        {
          /* Skip current and parent directory */
          if (strcmp(entry->name, ".") == 0 || strcmp(entry->name, "..") == 0)
            continue;

          /* Skip non-directories */
          if (entry->type != NF3DIR)
            continue;

          /* Print this entry */
          SCDC_TRACE("store: '" << entry->name << "'");
          SCDC_DATASET_OUTPUT_PRINTF_APPEND_(output, "%s|", entry->name);
        }
      }

      /*
       * Print files. The output has the following format:
       *
       *  <number of files>|<file entry 1>|<file entry 2>|...|
       *
       * where file entries have the format (<file format> is empty):
       *
       *  <filename>:<file format>:<size in bytes>
       *
       * Note that symbolic links and special files (devices, sockets, FIFOs...)
       * are ignored.
       */
      else
      {
        SCDC_TRACE("do_cmd_ls: listing entries of store '" << pwd << "'");

        std::ostringstream libnfs_path;
        libnfs_path << "/" << pwd;

        /* Open the specified directory */
        if (nfs_opendir(dataprov_nfs->_nfs_context, libnfs_path.str().c_str(),
          &dir) != 0)
        {
          result = "entries of store cannot be enumerated";
          SCDC_FAIL("do_cmd_ls: nfs_opendir('" << libnfs_path.str() << "') failed: " << dataprov_nfs->_libnfs_err());
          return false;
        }

        /* Count files first and print the number */
        scdcint_t entries = _count_fs_objects(dir, NF3REG);
        SCDC_DATASET_OUTPUT_PRINTF_(output, "%" scdcint_fmt "|", entries);

        /* Now print the names */
        while ((entry = nfs_readdir(dataprov_nfs->_nfs_context, dir)))
        {
          /* Skip current and parent directory */
          if (strcmp(entry->name, ".") == 0 || strcmp(entry->name, "..") == 0)
            continue;

          /* Skip non-files */
          if (entry->type != NF3REG)
            continue;

          // XXX: Entry format not supported yet
          const char *entry_format = "";

          /* Print this entry */
          SCDC_TRACE("entry: name: '" << entry->name << "', format: '" << entry_format << "', size: " << entry->size);
          SCDC_DATASET_OUTPUT_PRINTF_APPEND_(output, "%s:%s:%llu|", entry->name,
            entry_format, (unsigned long long)entry->size);
        }
      }

      /* Done */
      nfs_closedir(dataprov_nfs->_nfs_context, dir);

      return true;
    }


    bool do_cmd_put(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      /* Creates a store (subdirectory) or an entry within one (file) with the
         name given above. The file's data to be stored is brought into via the
         'input' parameter. */
      SCDC_TRACE("do_cmd_put: '" << params << "'");

      /* Must give a name */
      if (params.size() == 0)
      {
        result = "no filename specified";
        SCDC_FAIL(__func__ << ": " << result);
        return false;
      }

      scdc_dataprov_nfs2_store *dataprov_nfs = static_cast<scdc_dataprov_nfs2_store *>(dataprov);

      /* Admin mode? Create a directory with this name */
      int libnfs_ret;
      if (admin_mode)
      {
        SCDC_TRACE("do_cmd_put: creating store '" << params << "'");

        std::ostringstream libnfs_path;
        libnfs_path << "/" << params;

        /* Perform a nfs_mkdir() call only. If the directory already exists,
           it will fail with -EEXIST (this is hinted by the NFSv3 specification,
           and is guaranteed in POSIX 2008). */
        libnfs_ret = nfs_mkdir(dataprov_nfs->_nfs_context,
          libnfs_path.str().c_str());

        if (libnfs_ret != 0)
        {
          /* Does the directory already exist? */
          if (libnfs_ret == -EEXIST)
          {
            result = "store already exists";
            SCDC_FAIL("do_cmd_ls: store already exists");
            return false;
          }

          /* Any other error */
          result = "creating store failed";
          SCDC_FAIL("do_cmd_put: nfs_mkdir('" << libnfs_path.str() << "') failed: " << dataprov_nfs->_libnfs_err());
          return false;
        }

        /*
         * Until libnfs git commit adcfda60fa7a68b2274eb8b0fe1128ec34636979,
         * which makes the nfs_mkdir2() function available (not in any stable
         * release as of 2 June 2017) allowing to specify the permissions of the
         * created directory, all directories are created with 755 permissions.
         * Correct these to 777.
         */
        libnfs_ret = nfs_chmod(dataprov_nfs->_nfs_context,
          libnfs_path.str().c_str(), 0777);

        if (libnfs_ret != 0)
        {
          result = "changing new store permissions failed";
          SCDC_FAIL("do_cmd_put: nfs_chmod('" << libnfs_path.str() << "') to 777 failed: " << dataprov_nfs->_libnfs_err());
          return false;
        }

        /* Success, no output */

        return true;
      }

      /* No admin mode, must reside in a store in order to create files */
      if (pwd.size() == 0)
      {
        result = "no store specified";
        SCDC_FAIL(__func__ << ": " << result);
        return false;
      }

      /* Create/Overwrite a file with this name and transfer its content from
         the input */
      else
      {
        /* Need an input pointer for this operation */
        if (!input)
        {
          result = "no input available";
          SCDC_FAIL("do_cmd_put: " << result);
          return false;
        }

        SCDC_TRACE("do_cmd_put: creating entry '" << params << "'");

        /* Split the filename off */
        stringlist args(' ', params);
        std::string filename;
        args.front_pop(filename);

        /* Construct the target path */
        std::ostringstream dest_path;
        dest_path << "/" << pwd << "/" << filename;
        std::string dest_path_str = dest_path.str();

        SCDC_TRACE("do_cmd_put: dest_path: '" << dest_path_str << "'");

        /*
         * Try to open the target. Assume that the file exists; in this case, don't
         * overwrite its contents. If it fails because the file does not exist,
         * fall back to a write-only file creation.
         * This is because nfs_open() cannot create new files, unlike fopen() and
         * even open() on Unix.
         */
        struct nfsfh *nfs_file = 0;
        int libnfs_ret = nfs_open(dataprov_nfs->_nfs_context, dest_path_str.c_str(), O_RDWR, &nfs_file);

        /* Doesn't exist. Fall back to nfs_create(). */
        if (libnfs_ret == -ENOENT)
        {
          libnfs_ret = nfs_create(dataprov_nfs->_nfs_context, dest_path_str.c_str(), O_WRONLY, 0777, &nfs_file);

          if (libnfs_ret != 0)
          {
            result = string("entry can not be created: ") + dataprov_nfs->_libnfs_err();
            SCDC_FAIL("do_cmd_put: creating file '" << dest_path_str << "' failed: " << dataprov_nfs->_libnfs_err());

            return false;
          }

        } else if (libnfs_ret != 0)  /* Any other error; fail completely. */
        {
          result = string("entry can not be accessed: ") + dataprov_nfs->_libnfs_err();
          SCDC_FAIL("do_cmd_put: opening file '" << dest_path_str << "' failed: " << dataprov_nfs->_libnfs_err());

          return false;
        }

        size_t n = -1;

        do {

          /* Returns the number of bytes written by a single invocation of the write()
             system call, or -1 if the operation fails. */
          n = nfs_write(dataprov_nfs->_nfs_context, nfs_file, SCDC_DATASET_INOUT_BUF_CURRENT(input), SCDC_DATASET_INOUT_BUF_PTR(input));

          if (n < 0)
          {
            result = string("writing failed: ") + dataprov_nfs->_libnfs_err();
            SCDC_FAIL("do_cmd_put: writing failed: " << dataprov_nfs->_libnfs_err());
            break;
          }

        } while (input->next && input->next(input, 0) == SCDC_SUCCESS);

        nfs_close(dataprov_nfs->_nfs_context, nfs_file);

        return (n >= 0);
      }
    }


    bool do_cmd_get(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      /* Retrieves the content of the file specified as parameter. If no store
         is specified as current working directory, the command fails. */
      SCDC_TRACE("do_cmd_get: '" << params << "'");

      /* Must give a file name */
      if (params.empty())
      {
        result = "no filename specified";
        SCDC_FAIL(__func__ << ": " << result);
        return false;
      }

      /* Must have a store selected as working directory (and not be in admin
         mode) */
      else if (admin_mode || pwd.empty())
      {
        result = "no store selected";
        SCDC_FAIL(__func__ << ": " << result);
        return false;
      }

      /* Transfer the file's contents to the output */
      else
      {
        /* Need an output pointer for this operation */
        if (!output)
        {
          result = "no output available";
          SCDC_FAIL("do_cmd_get: no output available");
          return false;
        }

        SCDC_TRACE("do_cmd_get: getting entry '" << params << "'");

        /* Split the filename off */
        stringlist args(' ', params);
        std::string filename;
        args.front_pop(filename);

        /* Construct the target path */
        std::ostringstream dest_path;
        dest_path << "/" << pwd << "/" << filename;
        std::string dest_path_str = dest_path.str();

        SCDC_TRACE("do_cmd_get: dest_path: '" << dest_path_str << "'");

        scdc_dataprov_nfs2_store *dataprov_nfs = static_cast<scdc_dataprov_nfs2_store *>(dataprov);

        /* Check whether the file exists on the NFS share before attempting to
           read it. */
        struct nfs_stat_64 st;
        if (nfs_stat64(dataprov_nfs->_nfs_context, dest_path_str.c_str(),
          &st) != 0)
        {
          result = "entry does not exist or can not be accessed";
          SCDC_FAIL("do_cmd_get: nfs_stat64('" << dest_path_str << "') failed: " << dataprov_nfs->_libnfs_err());
          return false;
        }

        /* Open the target (which must exist). */
        struct nfsfh *nfs_file = 0;
        int libnfs_ret = nfs_open(dataprov_nfs->_nfs_context, dest_path_str.c_str(), O_RDONLY, &nfs_file);

        if (libnfs_ret != 0)
        {
          result = "entry can not be accessed: %s", dataprov_nfs->_libnfs_err();
          SCDC_FAIL("do_cmd_get: opening file '" << dest_path_str << "' failed: " << dataprov_nfs->_libnfs_err());
          return false;
        }

        dataset_output_next_data_t *data = new dataset_output_next_data_t();

        data->nfs_ctx = dataprov_nfs->_nfs_context;
        data->nfs_file = nfs_file;

        output->next = dataset_output_next;
        output->data = data;

        /* get first chunk of data */
        if (!output->next(output, 0))
        {
          result = "getting output failed";
          SCDC_FAIL("do_cmd_get: getting output failed");
          return false;
        }
      }

      return true;
    }


    bool do_cmd_rm(const std::string &params, scdc_dataset_input_t *input, scdc_dataset_output_t *output, scdc_result &result)
    {
      /* Removes the specified entry or store, depending on the current mode. */
      SCDC_TRACE("do_cmd_rm: '" << params << "'");

      /* Must give a file name */
      if (params.size() == 0)
      {
        result = "no filename specified";
        SCDC_FAIL(__func__ << ": " << result);
        return false;
      }

      /* Admin mode or no working directory selected: delete the specified store
         and all files within (equivalent to a recursive rm). */
      if (admin_mode || pwd.size() == 0)
      {
        SCDC_TRACE("do_cmd_rm: deleting store '" << params << "'");

        std::ostringstream libnfs_path;
        libnfs_path << "/" << params;

        /* Perform a recursive removal of the directory corresponding to the
           specified name. (If the name specifies a file instead, the opendir()
           call performed in this function will fail, so there's no need to
           check against.) */
        if (_rm_recursive(libnfs_path.str().c_str(), output, result))
        {
          return true;
        }

        /* We failed. _rm_recursive() wrote a reason into the output. */
        return false;
      }

      /* Normal operation: delete the specified file in the current store. */
      else
      {
        SCDC_TRACE("do_cmd_rm: deleting entry '" << params << "'");

        std::ostringstream libnfs_path;
        libnfs_path << "/" << pwd << "/" << params;
        std::string libnfs_path_str = libnfs_path.str();

        SCDC_TRACE("do_cmd_rm: libnfs_path: '" << libnfs_path_str << "'");

        scdc_dataprov_nfs2_store *dataprov_nfs = static_cast<scdc_dataprov_nfs2_store *>(dataprov);

        /*
         * The NFSv3 implementation says on the description of the
         * NFSPROC3_REMOVE RPC call used by nfs_unlink():
         *
         * "[...] REMOVE can be used to remove directories, [...]"
         *
         * Moreover, the POSIX 2008 description of unlink(), used to remove
         * files on a local file system, does not explicitly return a EISDIR
         * error like explicitly stated in the Linux man page for this function
         * if the target is a directory.
         *
         * Therefore, check if the file does exist (via nfs_stat()) and
         * in addition, whether it is not a directory.
         */
        struct nfs_stat_64 st;
        if (nfs_stat64(dataprov_nfs->_nfs_context, libnfs_path_str.c_str(),
          &st) != 0)
        {
          result = "entry does not exist or can not be accessed";
          SCDC_FAIL("do_cmd_rm: nfs_stat64('" << libnfs_path_str << "') failed: " << dataprov_nfs->_libnfs_err());
          return false;
        }
        if (S_ISDIR(st.nfs_mode))
        {
          result = "entry is a directory and cannot be removed";
          SCDC_FAIL(__func__ << ": " << result);
          return false;
        }

        /* Perform the unlink operation on this file. */
        if (nfs_unlink(dataprov_nfs->_nfs_context, libnfs_path_str.c_str()) != 0)
        {
          result = "entry cannot be deleted";
          SCDC_FAIL("do_cmd_rm: nfs_unlink('" << libnfs_path_str << "') failed: " << dataprov_nfs->_libnfs_err());
          return false;
        }

        /* File is gone. */

        return true;
      }
    }
};


#undef SCDC_LOG_PREFIX


#define SCDC_LOG_PREFIX  "dataprov-nfs: "


scdc_dataprov_nfs2_store::scdc_dataprov_nfs2_store()
  :scdc_dataprov("nfs"), _nfs_context(NULL)
{
}


bool scdc_dataprov_nfs2_store::open(const char *conf, scdc_args *args, scdc_result &result)
{
  SCDC_TRACE("open: conf: '" << conf << "'");

  bool ret = true;

  args = open_args_init(args);

  /* Get path to NFS share from arguments */
  const char *nfs_share_path;
  struct nfs_url *nfs_share_parsed = NULL;
  if (!args->get<const char *>(SCDC_ARGS_TYPE_CSTR, &nfs_share_path))
  {
    SCDC_ERROR("open: reading NFS share path");
    ret = false;
    goto do_quit;
  }

  /* Create the libnfs context */
  _nfs_context = nfs_init_context();
  if (_nfs_context == NULL)
  {
    SCDC_FAIL("open: creating libnfs context");
    ret = false;
    goto do_quit;
  }

  /*
   * Parsing some URLs with libnfs' parsing routines yields the following
   * results:
   *
   * nfs://127.0.0.1/
   *   Server: '127.0.0.1'
   *   Path:   ''
   *   File:   '/'
   *
   * nfs://127.0.0.1/a/b/c/
   *   Server: '127.0.0.1'
   *   Path:   '/a/b/c'
   *   File:   '/'
   *
   * nfs://127.0.0.1/a/b/c/file.txt
   *   Server: '127.0.0.1'
   *   Path:   '/a/b/c'
   *   File:   '/file.txt'
   *
   * Parse the URL (we need server and path anyway) and check if the file string
   * matches "/". This way, we can reject URLs to files, which are not allowed
   * when referencing a NFS share alone.
   */
  nfs_share_parsed = nfs_parse_url_full(_nfs_context, nfs_share_path);
  if (nfs_share_parsed == NULL)
  {
    SCDC_ERROR("open: parsing URL '" << nfs_share_path << "': "
      << _libnfs_err());
    ret = false;
    goto do_quit;
  }
  if (strcmp(nfs_share_parsed->file, "/") != 0)
  {
    SCDC_ERROR("open: URL '" << nfs_share_path << "' points to a file (it must "
      << "end with a slash)");
    ret = false;
    goto do_quit;
  }

  if (!scdc_dataprov::open(conf, args, result))
  {
    SCDC_FAIL("open: opening base");
    ret = false;

  } else
  {
    /*
     * The base provider is opened, try to mount the specified share using the
     * server and path components.
     */
    if (nfs_mount(_nfs_context, nfs_share_parsed->server,
      nfs_share_parsed->path) != 0)
    {
      SCDC_FAIL("open: mounting NFS share '" << nfs_share_parsed->path << "' "
        << "at " << nfs_share_parsed->server << ": " << _libnfs_err());
      ret = false;
      goto do_close;
    }

    dataset_cmds_add("pwd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs2_store::do_cmd_pwd));
    dataset_cmds_add("info", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs2_store::do_cmd_info));
    dataset_cmds_add("cd", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs2_store::do_cmd_cd));
    dataset_cmds_add("ls", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs2_store::do_cmd_ls));
    dataset_cmds_add("put", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs2_store::do_cmd_put));
    dataset_cmds_add("get", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs2_store::do_cmd_get));
    dataset_cmds_add("rm", static_cast<dataset_cmds_do_cmd_f>(&scdc_dataset_nfs2_store::do_cmd_rm));

    /* Success */
    SCDC_INFO("open: mounted NFS share '" << nfs_share_parsed->path << "' "
        << "at " << nfs_share_parsed->server);

do_close:
    if (!ret) scdc_dataprov::close(result);
  }

  open_args_clear();

do_quit:
  /* The parsed URL is no longer needed, even if we succeed */
  if (nfs_share_parsed != NULL) nfs_destroy_url(nfs_share_parsed);

  if (!ret)
  {
    /* Delete the libnfs context */
    if (_nfs_context != NULL) nfs_destroy_context(_nfs_context);

    open_args_release();
  }
  return ret;
}


bool scdc_dataprov_nfs2_store::close(scdc_result &result)
{
  SCDC_TRACE("close:");

  /* The share is unmounted by closing the context */

  bool ret = scdc_dataprov::close(result);

  /* Delete the libnfs context */
  nfs_destroy_context(_nfs_context);

  open_args_release();

  SCDC_TRACE("close: return: " << ret);

  return ret;
}


scdc_dataset *scdc_dataprov_nfs2_store::dataset_open(std::string &path, scdc_result &result)
{
  SCDC_TRACE("dataset_open: path: '" << path << "'");

  scdc_dataset_nfs2_store *dataset_nfs = new scdc_dataset_nfs2_store(this);

  SCDC_TRACE("dataset_open: return: " << dataset_nfs);

  return dataset_nfs;
}


bool scdc_dataprov_nfs2_store::dataset_close(scdc_dataset *dataset, scdc_result &result)
{
  SCDC_TRACE("dataset_close: dataset: " << dataset);

  bool ret = true;

  delete dataset;
  
  SCDC_TRACE("dataset_close: return: " << ret);

  return ret;
}


const char *scdc_dataprov_nfs2_store::_libnfs_err(void)
{
  return nfs_get_error(_nfs_context);
}


#undef SCDC_LOG_PREFIX
