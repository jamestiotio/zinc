/*******************************************************************************
FILE : callback_private.h

LAST MODIFIED : 20 March 2000

DESCRIPTION :
Macro definition for lists of callbacks between objects.
==============================================================================*/
#include "general/callback.h"
#include "general/debug.h"
#include "general/list_private.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

#define FULL_DECLARE_CALLBACK_TYPE( callback_type ) \
struct CALLBACK(callback_type) \
/***************************************************************************** \
LAST MODIFIED : 20 March 2000 \
\
DESCRIPTION : \
A callback. \
============================================================================*/ \
{ \
  CALLBACK_FUNCTION(callback_type) *function; \
	void *user_data; \
	int access_count; \
} /* CALLBACK(callback_type) */

#if defined (FULL_NAMES)
#define CALLBACK_OBJECT_AND_CALL_DATA( callback_type ) \
	callback_object_and_call_data_ ## callback_type
#else
#define CALLBACK_OBJECT_AND_CALL_DATA( callback_type ) \
  cbocd_ ## callback_type
#endif

#define FULL_DECLARE_CALLBACK_OBJECT_AND_CALL_DATA_TYPE( callback_type , \
  object_type , call_data_type) \
struct CALLBACK_OBJECT_AND_CALL_DATA(callback_type) \
/***************************************************************************** \
LAST MODIFIED : 20 March 2000 \
\
DESCRIPTION : \
Additional data - object and call_data structures sent with the callback. \
============================================================================*/ \
{ \
  object_type object; \
  call_data_type call_data; \
} /* struct CALLBACK_OBJECT_AND_CALL_DATA(callback_type) */

/*
Module functions
----------------
*/

#define DEFINE_CREATE_CALLBACK_FUNCTION( callback_type ) \
static struct CALLBACK(callback_type) *CREATE(CALLBACK(callback_type))( \
	CALLBACK_FUNCTION(callback_type) *function,void *user_data) \
/***************************************************************************** \
LAST MODIFIED : 20 March 2000 \
\
DESCRIPTION : \
Creates a callback containing <function> and <user_data>. \
============================================================================*/ \
{ \
  struct CALLBACK(callback_type) *callback; \
\
	ENTER(CREATE(CALLBACK(callback_type))); \
	if (function) \
	{ \
		if (ALLOCATE(callback,struct CALLBACK(callback_type),1)) \
		{ \
			callback->function=function; \
			callback->user_data=user_data; \
			callback->access_count=0; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CREATE(CALLBACK(" #callback_type ")).  Not enough memory"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CREATE(CALLBACK(" #callback_type ")).  Invalid argument(s)"); \
		callback=(struct CALLBACK(callback_type) *)NULL; \
	} \
	LEAVE; \
\
	return (callback); \
} /* CREATE(CALLBACK(callback_type)) */

#define DEFINE_DESTROY_CALLBACK_FUNCTION( callback_type ) \
static int DESTROY(CALLBACK(callback_type))( \
	struct CALLBACK(callback_type) **callback_address) \
/***************************************************************************** \
LAST MODIFIED : 20 March 2000 \
\
DESCRIPTION : \
Destroys the callback at <*callback_address>. \
============================================================================*/ \
{ \
  int return_code; \
\
	ENTER(DESTROY(CALLBACK(callback_type))); \
	if (callback_address&&(*callback_address)) \
	{ \
		DEALLOCATE(*callback_address); \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"DESTROY(CALLBACK(" #callback_type ")).  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* DESTROY(CALLBACK(callback_type)) */

#if defined (FULL_NAMES)
#define CALLBACK_CALL( callback_type ) callback_call_ ## callback_type
#else
#define CALLBACK_CALL( callback_type ) cbc_ ## callback_type
#endif

#define DEFINE_CALLBACK_CALL_FUNCTION( callback_type ) \
static int CALLBACK_CALL(callback_type)( \
	struct CALLBACK(callback_type) *callback,void *callback_data_void) \
/***************************************************************************** \
LAST MODIFIED : 20 March 2000 \
\
DESCRIPTION : \
Sends <callback> with the object and call_data in <callback_data>. \
============================================================================*/ \
{ \
	int return_code; \
  struct CALLBACK_OBJECT_AND_CALL_DATA(callback_type) *callback_data; \
\
	ENTER(CALLBACK_CALL(callback_type)); \
	if (callback&&callback->function&&(callback_data= \
		(struct CALLBACK_OBJECT_AND_CALL_DATA(callback_type) *) \
    callback_data_void)&&callback_data->object) \
	{ \
		(callback->function)(callback_data->object, \
			callback_data->call_data,callback->user_data); \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CALLBACK_CALL(" #callback_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CALLBACK_CALL(callback_type) */ \

#if defined (FULL_NAMES)
#define CALLBACK_MATCHES( callback_type ) callback_matches_ ## callback_type
#else
#define CALLBACK_MATCHES( callback_type ) cbm_ ## callback_type
#endif

#define DEFINE_CALLBACK_MATCHES_FUNCTION( callback_type ) \
static int CALLBACK_MATCHES(callback_type)( \
	struct CALLBACK(callback_type) *callback,void *other_callback_void) \
/***************************************************************************** \
LAST MODIFIED : 20 March 2000 \
\
DESCRIPTION : \
Returns true if the
Sends <callback> with the object and call_data in <callback_data>. \
============================================================================*/ \
{ \
	int return_code; \
  struct CALLBACK(callback_type) *other_callback; \
\
	ENTER(CALLBACK_MATCHES(callback_type)); \
	if (callback&& \
		(other_callback=(struct CALLBACK(callback_type) *)other_callback_void)) \
	{ \
		return_code=((callback->function == other_callback->function)&& \
			(callback->user_data == other_callback->user_data)); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CALLBACK_MATCHES(" #callback_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CALLBACK_MATCHES(callback_type) */ \

/*
Global functions
----------------
*/

#define DEFINE_CALLBACK_LIST_CALL_FUNCTION( callback_type , object_type , \
	call_data_type ) \
PROTOTYPE_CALLBACK_LIST_CALL_FUNCTION(callback_type,object_type,call_data_type)\
/***************************************************************************** \
LAST MODIFIED : 20 March 2000 \
\
DESCRIPTION : \
Calls every callback in <callback_list> with <object> and <call_data>. \
============================================================================*/ \
{ \
	int return_code; \
	struct CALLBACK_OBJECT_AND_CALL_DATA(callback_type) callback_data; \
\
	ENTER(CALLBACK_LIST_CALL(callback_type)); \
	if (callback_list&&object) \
	{ \
		callback_data.object=object; \
		callback_data.call_data=call_data; \
		return_code=FOR_EACH_OBJECT_IN_LIST(CALLBACK(callback_type))( \
			CALLBACK_CALL(callback_type),(void *)&callback_data,callback_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CALLBACK_LIST_CALL(" #callback_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CALLBACK_LIST_CALL(callback_type) */

#define DEFINE_CALLBACK_LIST_ADD_CALLBACK_FUNCTION( callback_type ) \
PROTOTYPE_CALLBACK_LIST_ADD_CALLBACK_FUNCTION(callback_type) \
/***************************************************************************** \
LAST MODIFIED : 20 March 2000 \
\
DESCRIPTION : \
Adds a callback = <function> + <user_data> to <callback_list>. \
============================================================================*/ \
{ \
	int return_code; \
  struct CALLBACK(callback_type) *callback; \
\
	ENTER(CALLBACK_LIST_ADD_CALLBACK(callback_type)); \
	if (callback_list&&function) \
	{ \
		if (callback=CREATE(CALLBACK(callback_type))(function,user_data)) \
		{ \
			if (FIRST_OBJECT_IN_LIST_THAT(CALLBACK(callback_type))( \
				CALLBACK_MATCHES(callback_type),(void *)callback,callback_list)) \
			{ \
				display_message(ERROR_MESSAGE,"CALLBACK_LIST_ADD_CALLBACK(" \
					#callback_type ").  Callback already exists in list"); \
				DESTROY(CALLBACK(callback_type))(&callback); \
				return_code=0; \
			} \
			else \
			{ \
				if (ADD_OBJECT_TO_LIST(CALLBACK(callback_type))( \
					callback,callback_list)) \
				{ \
					return_code=1; \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE,"CALLBACK_LIST_ADD_CALLBACK(" \
						#callback_type ").  Could not add callback to list"); \
					DESTROY(CALLBACK(callback_type))(&callback); \
					return_code=0; \
				} \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"CALLBACK_LIST_ADD_CALLBACK(" \
				#callback_type ").  Could not create callback"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"CALLBACK_LIST_ADD_CALLBACK(" \
			#callback_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CALLBACK_LIST_ADD_CALLBACK(callback_type) */

#define DEFINE_CALLBACK_LIST_REMOVE_CALLBACK_FUNCTION( callback_type ) \
PROTOTYPE_CALLBACK_LIST_REMOVE_CALLBACK_FUNCTION(callback_type) \
/***************************************************************************** \
LAST MODIFIED : 20 March 2000 \
\
DESCRIPTION : \
Removes a callback = <function> + <user_data> from <callback_list>. \
============================================================================*/ \
{ \
	int return_code; \
  struct CALLBACK(callback_type) callback,*existing_callback; \
\
	ENTER(CALLBACK_LIST_REMOVE_CALLBACK(callback_type)); \
	if (callback_list&&function) \
	{ \
		callback.function=function; \
		callback.user_data=user_data; \
		if (existing_callback=FIRST_OBJECT_IN_LIST_THAT(CALLBACK(callback_type))( \
			CALLBACK_MATCHES(callback_type),(void *)&callback,callback_list)) \
		{ \
			if (REMOVE_OBJECT_FROM_LIST(CALLBACK(callback_type))( \
				existing_callback,callback_list)) \
			{ \
				return_code=1; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE,"CALLBACK_LIST_REMOVE_CALLBACK(" \
					#callback_type ").  Could not remove callback from list"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"CALLBACK_LIST_REMOVE_CALLBACK(" \
				#callback_type ").  Could not find callback in list"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"CALLBACK_LIST_REMOVE_CALLBACK(" \
			#callback_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CALLBACK_LIST_REMOVE_CALLBACK(callback_type) */

#define FULL_DECLARE_CALLBACK_TYPES( callback_type , object_type , \
  call_data_type ) \
FULL_DECLARE_CALLBACK_TYPE(callback_type); \
FULL_DECLARE_LIST_TYPE(CALLBACK(callback_type)); \
FULL_DECLARE_CALLBACK_OBJECT_AND_CALL_DATA_TYPE(callback_type,object_type, \
  call_data_type)

#define DEFINE_CALLBACK_MODULE_FUNCTIONS( callback_type ) \
DEFINE_CREATE_CALLBACK_FUNCTION(callback_type) \
DEFINE_DESTROY_CALLBACK_FUNCTION(callback_type) \
DECLARE_OBJECT_FUNCTIONS(CALLBACK(callback_type)) \
DEFINE_CALLBACK_CALL_FUNCTION(callback_type) \
DEFINE_CALLBACK_MATCHES_FUNCTION(callback_type)

#define DEFINE_CALLBACK_FUNCTIONS( callback_type , object_type , \
	call_data_type ) \
DECLARE_LIST_FUNCTIONS(CALLBACK(callback_type)) \
DEFINE_CALLBACK_LIST_CALL_FUNCTION( callback_type , object_type , \
	call_data_type ) \
DEFINE_CALLBACK_LIST_ADD_CALLBACK_FUNCTION(callback_type) \
DEFINE_CALLBACK_LIST_REMOVE_CALLBACK_FUNCTION(callback_type)
