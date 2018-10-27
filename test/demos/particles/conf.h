
#ifndef __CONF_H__
#define __CONF_H__


/* component selection */
#if USE_COMP_ALL
# ifndef USE_COMPU
#  define USE_COMPU  1
# endif
# ifndef USE_COMPD
#  define USE_COMPD  1
# endif
# ifndef USE_COMPG
#  define USE_COMPG  1
# endif
# ifndef USE_COMPL
#  define USE_COMPL  1
# endif
# ifndef USE_COMPC
#  define USE_COMPC  1
# endif
# ifndef USE_COMPI
#  define USE_COMPI  1
# endif
#endif


/* component distribution */

/* local components */
#define COMPL_SCDC_URI  "scdc:/compL"
#define COMPD_SCDC_URI  "scdc:/compD"
#define COMPC_SCDC_URI  "scdc:/compC"
#define COMPI_SCDC_URI  "scdc:/compI"
#define COMPG_SCDC_URI  "scdc:/compG"

/* remote components  */
// #define COMPC_SCDC_URI  "scdc+uds:/compC"


#endif /* __CONF_H__ */
