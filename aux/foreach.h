#ifndef _AUX_FOREACH_H_
#define _AUX_FOREACH_H_

#define AR_FOREACH(array,len,elt)   for(elt=(array);elt<((array)+(len));elt++)
#define AR_SEARCH(array,len,elt,cmp) ({ \
            void *ret = NULL;            \
            AR_FOREACH(array,len,elt) {  \
                if(cmp) {                \
                    ret = (void*) elt;   \
                    break;               \
                }                        \
            }                            \
            ret;                         \
        })

#endif
