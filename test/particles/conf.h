
#ifndef __CONF_H__
#define __CONF_H__


/* component selection */
#if USE_COMP_ALL
# define USE_COMPU  1
# define USE_COMPD  1
# define USE_COMPG  1
# define USE_COMPL  1
# define USE_COMPC  1
# define USE_COMPI  1
#endif


/* component distribution */

/* local components */
#define COMPL_SCDC_URI  "scdc:/compL"
#define COMPD_SCDC_URI  "scdc:/compD"
// #define COMPC_SCDC_URI  "scdc:/compC"
#define COMPI_SCDC_URI  "scdc:/compI"
#define COMPG_SCDC_URI  "scdc:/compG"

/* remote components  */
#define COMPC_SCDC_URI  "scdc+uds:/compC"


#endif /* __CONF_H__ */
