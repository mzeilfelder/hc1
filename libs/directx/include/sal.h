// Workaround by Michael Zeilfelder
// sal annotations are not part of the windows sdk - still included by dsound.h - yay :-(
// Don't need 'em - so just defining them away
// You can use this file with any license you want as long as you don't make me responsible for any troubles it causes.

#ifndef SAL_BY_MICHA_Z_H_
#define SAL_BY_MICHA_Z_H_

#define __in 
#define __out
#define __in_opt
#define __deref_out
#define __null 
#define __out_opt
#define __out_bcount_opt(x)
#define __deref_out_bcount(x)
#define __deref_opt_out_bcount(x)
#define __in_bcount(x)
#define __in_bcount_opt(x)
#define __in_ecount_opt(x)
#define __out_ecount_opt(x)
#define __out_ecount(x)
#define __in_ecount(x)
#define __out_bcount(x)

#endif 
