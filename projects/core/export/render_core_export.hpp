
#ifndef RENDER_CORE_EXPORT_H
#define RENDER_CORE_EXPORT_H

#ifdef RENDER_CORE_STATIC_DEFINE
#define RENDER_CORE_EXPORT
#define RENDER_CORE_NO_EXPORT
#else
#ifndef RENDER_CORE_EXPORT
#ifdef render_core_EXPORTS
/* We are building this library */
#define RENDER_CORE_EXPORT
#else
/* We are using this library */
#define RENDER_CORE_EXPORT
#endif
#endif

#ifndef RENDER_CORE_NO_EXPORT
#define RENDER_CORE_NO_EXPORT
#endif
#endif

#ifndef RENDER_CORE_DEPRECATED
#define RENDER_CORE_DEPRECATED __declspec(deprecated)
#endif

#ifndef RENDER_CORE_DEPRECATED_EXPORT
#define RENDER_CORE_DEPRECATED_EXPORT RENDER_CORE_EXPORT RENDER_CORE_DEPRECATED
#endif

#ifndef RENDER_CORE_DEPRECATED_NO_EXPORT
#define RENDER_CORE_DEPRECATED_NO_EXPORT                                       \
  RENDER_CORE_NO_EXPORT RENDER_CORE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#ifndef RENDER_CORE_NO_DEPRECATED
#define RENDER_CORE_NO_DEPRECATED
#endif
#endif

#endif /* RENDER_CORE_EXPORT_H */
