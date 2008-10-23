#ifndef _API_H_
#define _API_H_

#ifdef _WIN32
#ifdef AVG_PLUGIN
#define AVG_API __declspec(dllimport)
#define AVG_TEMPLATE_API
#else
#define AVG_API __declspec(dllexport)
#define AVG_TEMPLATE_API __declspec(dllexport)
#endif
#else // _WIN32
#define AVG_API
#define AVG_TEMPLATE_API
#endif

#endif