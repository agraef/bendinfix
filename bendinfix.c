
// To compile on Linux: gcc --shared -fPIC -o bendinfix.pd_linux bendinfix.c

#include <m_pd.h>

static t_class *bendinfix_class;
static int *legacy;
static t_symbol **pdinfo_l2ork_version_s;
static void (*nw_gui_vmess)(const char *sel, char *fmt, ...);

typedef struct _bendinfix {
  t_object  x_obj;
} t_bendinfix;

void bendinfix_float(t_bendinfix *x, t_floatarg f)
{
  // vanilla default:
  t_float g = 8192;
  // exported symbols by the different Pd flavors:
  // legacy => pd-l2ork and purr-data
  // nw_gui_vmess => purr-data only
  // pdinfo_l2ork_version_s => purr-data with revised vanilla-compatible
  // bendin implementation
  if (legacy && !pdinfo_l2ork_version_s)
    // we always have a signed bendin with classic pd-l2ork (!nw_gui_vmess)
    // for purr-data <= 2.13.0 (!pdinfo_l2ork_version_s) bendin is signed,
    // unless -legacy flag is set
    g = !nw_gui_vmess?0:*legacy?8192:0;
  outlet_float(x->x_obj.ob_outlet, f-g);
}

void *bendinfix_new(void)
{
  t_bendinfix *x = (t_bendinfix *)pd_new(bendinfix_class);
  outlet_new(&x->x_obj, &s_float);
  return (void *)x;
}

#ifdef WIN32
#include <windows.h>
#else
#define __USE_GNU // to get RTLD_DEFAULT
#include <dlfcn.h> // for dlsym
#ifndef RTLD_DEFAULT
/* If RTLD_DEFAULT still isn't defined then just passing NULL will hopefully
   do the trick. */
#define RTLD_DEFAULT NULL
#endif
#endif

void bendinfix_setup(void) {
  bendinfix_class = class_new(gensym("bendinfix"),
        (t_newmethod)bendinfix_new,
        0, sizeof(t_bendinfix),
        CLASS_DEFAULT, 0);
  class_addfloat(bendinfix_class, bendinfix_float);
#ifdef WIN32
  legacy = (void*)GetProcAddress(GetModuleHandle("pd.dll"), "sys_legacy");
  pdinfo_l2ork_version_s = (void*)GetProcAddress(GetModuleHandle("pd.dll"), "pdinfo_l2ork_version_s");
  nw_gui_vmess = (void*)GetProcAddress(GetModuleHandle("pd.dll"), "gui_vmess");
#else
  legacy = dlsym(RTLD_DEFAULT, "sys_legacy");
  pdinfo_l2ork_version_s = dlsym(RTLD_DEFAULT, "pdinfo_l2ork_version_s");
  nw_gui_vmess = dlsym(RTLD_DEFAULT, "gui_vmess");
#endif
}
