#include "oob_config.h"
#include "string.h"

void page_ecc_gen(unsigned char *pagebuf, 
		  enum page_type pagetype, 
		  enum ecc_type ecctype);
extern void page_random_gen(unsigned char *pagebuf, 
			    enum page_type pagetype, 
			    enum ecc_type ecctype, 
			    int pageindex,
			    int oobsize);
