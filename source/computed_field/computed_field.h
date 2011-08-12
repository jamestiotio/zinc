/*******************************************************************************
FILE : computed_field.h

LAST MODIFIED : 18 April 2005

DESCRIPTION :
A Computed_field is an abstraction of an FE_field. For each FE_field there is
a wrapper Computed_field automatically generated that can be called on to
evaluate the field in an element or node. The interface for evaluating
Computed_fields is much simpler than for FE_field, since they hide details of
caching of FE_element_field_values, for example. Their main benefit is in
allowing new types of fields to be defined as functions of other fields and
source information, such as scale, offset, magnitude, gradient,
coordinate transformations etc., thus providing cmgui with the ability to
provide customised features without cluttering dialogs such as the graphical
element editor. Also, finite_element_to_graphics_object is greatly simplified
because it can assume all coordinate and other fields are in rectangular
cartesian coordinates - if they are not already, the rest of the program can
make a simple wrapper computed_field effecting this change. A positive
consequence of this change is that this file should in time contain the only
code for invoking coordinate transformations in the entire program.

In addition to these functional benefits, computed fields cache the last
position they were evaluated at and the values and derivatives of the field at
that point so that if values at that point are requested again they can be
immediately returned. This allows functions using a number of fields, some
possibly depending on each other to get the maximum benefit out of values that
have already been calculated, without requiring complex logic for determining
if a value is already known.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (COMPUTED_FIELD_H)
#define COMPUTED_FIELD_H

#include "api/cmiss_field.h"
#include "command/parser.h"
#include "finite_element/finite_element.h"
#include "general/geometry.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "user_interface/message.h"
#ifdef __cplusplus
#include "computed_field/field_location.hpp"
#endif /* __cplusplus */

struct Field_location;
struct Cmiss_region;

/*
The Cmiss_computed_field which is Public is currently the same object as the
cmgui internal Computed_field.  The Public interface is contained in
api/cmiss_field.h however most of the functions come directly from
this module.  So that these functions match the public declarations the
functions are given their public names.
*/

/* Convert to use external field object name */
#define Computed_field Cmiss_field

/* Convert the functions that have identical interfaces */
#define Computed_field_get_number_of_components \
	Cmiss_field_get_number_of_components
#define Computed_field_set_type Cmiss_field_set_type

/*
Global types
------------
*/

struct Computed_field;
/*******************************************************************************
LAST MODIFIED : 24 December 1998

DESCRIPTION :
==============================================================================*/

DECLARE_LIST_TYPES(Computed_field);

DECLARE_MANAGER_TYPES(Computed_field);

struct Computed_field_package;
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Contains all information for editing and maintaining Computed_fields, including
the MANAGER(Computed_field).
struct Computed_field_package is private.
???RC Make macro PACKAGE(Computed_field) etc.?
==============================================================================*/

struct List_Computed_field_commands_data
{
	const char *command_prefix;
	int listed_fields;
	struct LIST(Computed_field) *computed_field_list;
	struct MANAGER(Computed_field) *computed_field_manager;
}; /* struct List_Computed_field_commands_data */

struct Computed_field_conditional_data
{
	struct Computed_field *conditional_field;
	FE_value time;
}; /* struct Computed_field_conditional_data */

/***************************************************************************//**
 * Stores data and settings needed to create field, including the region to add
 * it to.
 */
struct Cmiss_field_module;

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

PROTOTYPE_OBJECT_FUNCTIONS(Computed_field);
/*PROTOTYPE_COPY_OBJECT_FUNCTION(Computed_field);*/
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Computed_field);

PROTOTYPE_LIST_FUNCTIONS(Computed_field);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Computed_field,name,const char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Computed_field,name,const char *);
PROTOTYPE_MANAGER_FUNCTIONS(Computed_field);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Computed_field,name,const char *);

int Computed_field_clear_cache(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 10 February 1999

DESCRIPTION :
Clears any caching of elements/values of <field> and any fields it refers to.
Must call this function for each field after you have used functions
Computed_field_evaluate_in_element or Computed_field_evaluate_at_node and they
are possibly not going to be called again for some time.
==============================================================================*/

int Computed_field_is_defined_in_element(struct Computed_field *field,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 13 May 1999

DESCRIPTION :
Returns true if <field> can be calculated in <element>. If the field depends on
any other fields, this function is recursively called for them.
==============================================================================*/

int Computed_field_is_defined_in_element_conditional(
	struct Computed_field *field,void *element_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Manager conditional function version of Computed_field_is_defined_in_element.
==============================================================================*/

int Computed_field_is_true_in_element(struct Computed_field *field,
	struct FE_element *element,FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns true if <field> is determined to be "true" at the centre of <element>.
This is currently that the field is defined and any of the components are non zero.
==============================================================================*/

int FE_element_Computed_field_is_not_true_iterator(struct FE_element *element,
	void *computed_field_conditional_data_void);
/*******************************************************************************
LAST MODIFIED : 5 December 2002

DESCRIPTION :
Iterator version of NOT Computed_field_is_true_in_element.
==============================================================================*/

int Computed_field_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 13 May 1999

DESCRIPTION :
Returns true if <field> can be calculated at <node>. If the field depends on
any other fields, this function is recursively called for them.
==============================================================================*/

int Computed_field_is_defined_at_node_conditional(struct Computed_field *field,
	void *node_void);
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Computed_field conditional function version of
Computed_field_is_defined_at_node.
==============================================================================*/

int Computed_field_is_in_list(struct Computed_field *field,
	void *field_list_void);
/*******************************************************************************
LAST MODIFIED : 5 February 2003

DESCRIPTION :
Computed_field conditional/iterator function returning true if <field> is in the
computed <field_list>.
==============================================================================*/

int Computed_field_is_true_at_node(struct Computed_field *field,
	struct FE_node *node, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Returns true if <field> is determined to be "true" at <node>.  This is currently
that the field is defined and any of the components are non zero.
==============================================================================*/

int FE_node_Computed_field_is_not_true_iterator(struct FE_node *node,
	void *fe_node_computed_field_conditional_data_void);
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Iterator version of NOT Computed_field_is_true_at_node.
==============================================================================*/

int FE_node_has_Computed_field_defined(struct FE_node *node,void *field_void);
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
FE_node conditional function version of Computed_field_is_defined_at_node.
==============================================================================*/

int Computed_field_depends_on_Computed_field(struct Computed_field *field,
	struct Computed_field *other_field);
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Returns true if the two fields are identical or one of the source_fields of
<field> is <other_field> or depends on it.

This function is used by MANAGER_COPY_WITHOUT_IDENTIFIER to ensure that the
destination field will not depend on itself, thus leading to an infinite loop,
and also to prevent copying a field over itself.

Parts of the program receiving manager messages for Computed_fields should call
this function with the field=field in use and other_field=modified field to
determine if the field in use needs updating.
==============================================================================*/

int Computed_field_or_ancestor_satisfies_condition(struct Computed_field *field,
	LIST_CONDITIONAL_FUNCTION(Computed_field) *conditional_function,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 5 February 2003

DESCRIPTION :
Returns true if <field> satisfies <conditional_function> with <user_data>. If
not, recursively calls this function for each of its source fields until one
satisfies the function for a true result, or all have failed for false.
==============================================================================*/

int Computed_field_for_each_ancestor(struct Computed_field *field,
	LIST_ITERATOR_FUNCTION(Computed_field) *iterator_function, void *user_data);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
For <field> and all of its source Computed_fields, calls <iterator_function>
with <user_data>. Iteration stops if a single iterator_function call returns 0.
==============================================================================*/

/***************************************************************************//**
 * Prescribes a location in an element for field evaluation or assignment with
 * the cache. Variant with optional prescribed parent element.
 * Note: replaces any other spatial location in cache (e.g. node.) but time
 * is unchanged.
 *
 * @param cache  The field cache to set the location in.
 * @param element  The element the location is in. Must belong to same region
 * as cache.
 * @param number_of_chart_coordinates  The size of the chart_coordinates array,
 * checked to be not less than the element dimension.
 * @param chart_coordinates  Location in element chart. Value is not checked;
 * caller is responsible for supplying locations within the bounds of the
 * element shape.
 * @param top_level_element  Optional highest dimensional parent element to
 * inherit fields from.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_cache_set_mesh_location_with_parent(
	Cmiss_field_cache_id cache, Cmiss_element_id element,
	int number_of_chart_coordinates, double *chart_coordinates,
	Cmiss_element_id top_level_element);

/***************************************************************************//**
 * Clears only values cache for field and its sources, not deeper type-specific
 * cache. Useful for doing quick evaluations at different locations.
 * Internal function.
 *
 * @param cache  The field cache to modify.
 * @param field  The field whose values cache is to be cleared, along with all
 * its source fields, recursively.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_cache_clear_field_values(Cmiss_field_cache_id cache,
	Cmiss_field_id field);

/***************************************************************************//**
 * Fully clears cache of supplied field, its source fields and any fields that
 * depend on its values.
 * Internal function.
 *
 * @param cache  The field cache to modify.
 * @param field  The field whose values cache is to be cleared, along with all
 * its source and dependent fields, recursively.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_cache_invalidate_field(Cmiss_field_cache_id field_cache,
	Cmiss_field_id field);

/***************************************************************************//**
 * Internal function similar to Cmiss_field_assign_real but which only assigns
 * the value in the field_cache, propagating it back to its source fields. This
 * means subsequent evaluations at the same cache location will return the
 * values if set. Changing the cache location clears these temporary cached
 * values.
 *
 * @param cache  Store of location to assign at and intermediate field values.
 * @param field  The field to assign real values to in the cache.
 * @param number_of_values  Size of values array. Checked that it equals or
 * exceeds the number of components of field.
 * @param values  Array of real values to assign to field.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_cache_assign_field_real(Cmiss_field_cache_id cache,
	Cmiss_field_id field, int number_of_values, double *values);

/***************************************************************************//**
 * Internal function.
 * Note: no warnings if not evaluated so can be used for is_defined
 *
 * @return  1 if any field component value is non zero with a small tolerance,
 * otherwise 0.
 */
int Cmiss_field_evaluate_boolean(Cmiss_field_id field,
	Cmiss_field_cache_id cache);

char *Computed_field_evaluate_as_string_in_element(
	struct Computed_field *field,int component_number,
	struct FE_element *element,FE_value *xi,FE_value time,
	struct FE_element *top_level_element);
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
Returns a string representing the value of <field>.<component_number> at
<element>:<xi>. If <field> is a FINITE_ELEMENT wrapper without a value type of
FE_VALUE_VALUE, requests the string from it, otherwise calls
Computed_field_evaluate_cache_in_element and converts the value for
<component_number> to a string (since result may already be in cache).

Use -1 as the <component_number> if you want all the components.

The <top_level_element> parameter has the same use as in
Computed_field_evaluate_cache_in_element.

Some basic field types such as CMISS_NUMBER have special uses in this function.
It is up to the calling function to DEALLOCATE the returned string.
???RC.  Allow derivatives to be evaluated as string too?
==============================================================================*/

int Computed_field_evaluate_in_element(struct Computed_field *field,
	struct FE_element *element,FE_value *xi,FE_value time,
	struct FE_element *top_level_element,FE_value *values,FE_value *derivatives);
/*******************************************************************************
LAST MODIFIED : 29 June 1999

DESCRIPTION :
Returns the values and derivatives (if <derivatives> != NULL) of <field> at
<element>:<xi>, if it is defined over the element. Can verify this in advance
by calling function Computed_field_defined_in_element. Each <field> has a cache
for storing its values and derivatives, which is allocated and filled by a call
to Computed_field_evaluate_cache_in_element, then copied to the <values> and
<derivatives> arrays.

The optional <top_level_element> may be supplied for the benefit of this or
any source fields that may require calculation on it instead of a face or line.
FIBRE_AXES and GRADIENT are examples of such fields, since they require
top-level coordinate derivatives. The term "top_level" refers to an ultimate
parent element for the face or line, eg. the 3-D element parent to 2-D faces.
If no such top level element is supplied and one is required, then the first
available parent element will be chosen - if the user requires a top-level
element in the same group as the face or with the face on the correct side,
then they should supply the top_level_element here.

The <values> and <derivatives> arrays must be large enough to store all the
values and derivatives for this field in the given element, ie. values is
number_of_components in size, derivatives has the element dimension times the
number_of_components
==============================================================================*/

char *Computed_field_evaluate_as_string_at_node(struct Computed_field *field,
	int component_number, struct FE_node *node, FE_value time);
/*******************************************************************************
LAST MODIFIED : 3 July 2000

DESCRIPTION :
Returns a string describing the value/s of the <field> at the <node>. If the
field is based on an FE_field but not returning FE_values, it is asked to supply
the string. Otherwise, a string built up of comma separated values evaluated
for the field in Computed_field_evaluate_cache_at_node. The FE_value exception
is used since it is likely the values are already in the cache in most cases,
or can be used by other fields again if calculated now.
The <component_number> indicates which component to calculate.  Use -1 to 
create a string which represents all the components.
Some basic field types such as CMISS_NUMBER have special uses in this function.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/

int Computed_field_evaluate_at_node(struct Computed_field *field,
	struct FE_node *node, FE_value time, FE_value *values);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Returns the <values> of <field> at <node> if it is defined there. Can verify
this in advance by calling function Computed_field_defined_at_node. Each <field>
has a cache for storing its values and derivatives, which is allocated and the
field->values array filled by a call to Computed_field_evaluate_cache_at_node,
which is then copied to <values> by this function. Derivatives may only be
calculated in elements, however, the field->derivatives array is allocated here
with field->values since Computed_field_evaluate_cache_in_element expects both
to be allocated together.

The <values> array must be large enough to store as many FE_values as there are
number_of_components.
==============================================================================*/

int Cmiss_field_evaluate_at_field_coordinates(
	Cmiss_field_id field,
	Cmiss_field_id reference_field, int number_of_input_values,
	double *input_values, double time, double *values);
/*******************************************************************************
LAST MODIFIED : 25 March 2008

DESCRIPTION :
Returns the <values> of <field> at the location of <input_values>
with respect to the <reference_field> if it is defined there.

The <values> array must be large enough to store as many FE_values as there are
number_of_components.
==============================================================================*/

int Computed_field_set_values_at_node(struct Computed_field *field,
	struct FE_node *node, FE_value time, FE_value *values);
/*******************************************************************************
LAST MODIFIED : 21 April 2005

DESCRIPTION :
Sets the <values> of the computed <field> at <node>. Only certain computed field
types allow their values to be set. Fields that deal directly with FE_fields eg.
FINITE_ELEMENT and NODE_VALUE fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the same
function for it. If a field has more than one source field, eg. RC_VECTOR, it
can in many cases still choose which one is actually being changed, for example,
the 'vector' field in this case - coordinates should not change. This process
continues until the actual FE_field values at the node are changed or a field
is reached for which its calculation is not reversible, or is not supported yet.
???RC Note that some functions are not reversible in this way.
==============================================================================*/

#ifdef __cplusplus
int Computed_field_evaluate_at_location(struct Computed_field *field,
	Field_location& loc, FE_value *values, FE_value *derivatives = 0);
#endif /* __cplusplus */

int Computed_field_get_values_in_element(struct Computed_field *field,
	struct FE_element *element, int *number_in_xi, FE_value time,
	FE_value **values);
/*******************************************************************************
LAST MODIFIED : 27 October 2004

DESCRIPTION :
Companion function to Computed_field_set_values_in_element.
Returns the <field> calculated at the corners of the <number_in_xi> cells,
evenly spaced in xi, over the element. <values> should be allocated with enough
space for number_of_components * product of number_in_xi+1 in each element
direction, the returned values cycling fastest through number of grid points in
xi1, number of grid points in xi2 etc. and lastly components.
It is up to the calling function to deallocate the returned values.
==============================================================================*/

int Computed_field_set_values_in_element(struct Computed_field *field,
	struct FE_element *element, FE_value *xi, FE_value time,
	FE_value *values);
/*******************************************************************************
LAST MODIFIED : 14 October 2005

DESCRIPTION :
Sets the <values> of the computed <field> at <xi> in the <element>. Only certain
computed field types allow their values to be set. Fields that deal directly
with FE_fields eg. FINITE_ELEMENT fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the
same function for it. If a field has more than one source field, eg. RC_VECTOR,
it can in many cases still choose which one is actually being changed, for
example, the 'vector' field in this case - coordinates should not change. This
process continues until the actual FE_field values in the element are changed or
a field is reached for which its calculation is not reversible, or is not
supported yet.
==============================================================================*/

int Computed_field_get_native_discretization_in_element(
	struct Computed_field *field,struct FE_element *element,int *number_in_xi);
/*******************************************************************************
LAST MODIFIED : 28 October 1999 

DESCRIPTION :
If the <field> or its source field is grid-based in <element>, returns in
<number_in_xi> the numbers of finite difference cells in each xi-direction of
<element>. Note that this number is one less than the number of grid points in
each direction. <number_in_xi> should be allocated with at least as much space
as the number of dimensions in <element>, but is assumed to have no more than
MAXIMUM_ELEMENT_XI_DIMENSIONS so that
int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] can be passed to this function.
Returns 0 with no errors if the field is not grid-based.
==============================================================================*/

int Computed_field_get_native_resolution(struct Computed_field *field,
        int *dimension, int **sizes, 
	struct Computed_field **texture_coordinate_field);
/*******************************************************************************
LAST MODIFIED : 03 February 2005

DESCRIPTION :
Gets the <dimension>, <sizes>, <minimums>, <maximums> and <texture_coordinate_field> from
the <field>. These parameters will be used in image processing.

==============================================================================*/

int Computed_field_get_number_of_components(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 23 December 1998

DESCRIPTION :
==============================================================================*/

char *Computed_field_get_component_name(struct Computed_field *field,
	int component_no);
/*******************************************************************************
LAST MODIFIED : 29 December 1998

DESCRIPTION :
Returns an allocated string containing the name of <component_no> of <field>.
Name of the component depends on the type of the Computed_field; for example,
COMPUTED_FIELD_FINITE_ELEMENT gets component names from the FE_field it is wrapping. Default
names are made out of the character form of the component_no+1, eg, 1 -> "2".
It is up to the calling function to deallocate the returned string.
==============================================================================*/

struct Coordinate_system *Computed_field_get_coordinate_system(
	struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 19 January 1999

DESCRIPTION :
Returns the coordinate system <field> is to be interpreted under. See function
Computed_field_set_coordinate_system for further details.
==============================================================================*/

int Computed_field_set_coordinate_system(struct Computed_field *field,
	struct Coordinate_system *coordinate_system);
/*******************************************************************************
LAST MODIFIED : 19 January 1999

DESCRIPTION :
Sets the coordinate system <field> is to be interpreted under. Note the careful
choice of words here: the coordinate system merely tells the rest of the program
what needs to be done to transform the field values into any other coordinate
system. It does not have to be "correct" for the values in the field, eg. you
can describe prolate spheroidal values as RC to "open out" the heart model.
<focus> must be given with prolate and oblate spheroidal coordinate systems.
==============================================================================*/

const char *Computed_field_get_type_string(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 24 January 2002

DESCRIPTION :
Returns the string which identifies the type.
The calling function must not deallocate the returned string.
==============================================================================*/

int Computed_field_has_3_components(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Conditional function returning true if <field> has exactly three components.
==============================================================================*/

int Computed_field_has_4_components(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Conditional function returning true if <field> has exactly four components.
==============================================================================*/

int Computed_field_has_16_components(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 21 February 2008

DESCRIPTION :
Iterator/conditional function returning true if <field> has exactly sixteen
components - useful for selecting transformation matrix.
==============================================================================*/

int Computed_field_has_up_to_4_numerical_components(
	struct Computed_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has up to 4 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/

int Computed_field_has_n_components(struct Computed_field *field,
	void *components_ptr_void);
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Iterator/conditional function returning true if <field> has the same number of
components as that specified by <components_ptr_void>.
==============================================================================*/

int Computed_field_has_at_least_2_components(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Iterator/conditional function returning true if <field> has at least 2 components.
==============================================================================*/

/*******************************************************************************
 * @return  1 if field returns mesh_location value type.
 */
int Computed_field_has_value_type_mesh_location(struct Computed_field *field,
	void *dummy_void);

int Computed_field_has_numerical_components(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> returns numerical components.
Note that whether the numbers were integer, FE_value or double, they may be
returned as FE_value when evaluated.
==============================================================================*/

int Computed_field_is_scalar(struct Computed_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has 1 component and it is
numerical.
==============================================================================*/

int Computed_field_has_up_to_3_numerical_components(
	struct Computed_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has up to 3 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/

int Computed_field_has_up_to_4_numerical_components(
	struct Computed_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has up to 4 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/

int Computed_field_has_multiple_times(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Conditional function returning true if <field> depends on time.
==============================================================================*/

int Computed_field_is_orientation_scale_capable(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 February 1999

DESCRIPTION :
Conditional function returning true if the field can be used to orient or scale
glyphs. Generally, this means it has 1,2,3,4,6 or 9 components, where:
1 = scalar (no vector, isotropic scaling).
2 = 1 2-D vector (2nd axis is normal in plane, 3rd is out of 2-D plane);
3 = 1 3-D vector (orthogonal 2nd and 3rd axes are arbitrarily chosen);
4 = 2 2-D vectors (3rd axis taken as out of 2-D plane);
6 = 2 3-D vectors (3rd axis found from cross product);
9 = 3 3-D vectors = complete definition of 3 axes.
???RC.  Include coordinate system in test?
==============================================================================*/

int Computed_field_is_stream_vector_capable(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Conditional function returning true if the field is suitable for 3-D streamline
tracking. This means it has either 3, 6 or 9 components (with 3 components per
vector), or has a FIBRE coordinate_system, meaning it can be wrapped to produce
9-component fibre_axes.  Also now supports 2 components for use with a 2
component coordinate field.
The number of components controls how the field is interpreted:
3 = 1 3-D vector (lateral direction and normal worked out from curl of field);
6 = 2 3-D vectors (2nd vector is lateral direction. Stream ribbon normal found
    from cross product);
9 = 3 3-D vectors (2nd vector is lateral direction; 3rd vector is stream ribbon
    normal).
==============================================================================*/

int Computed_field_find_element_xi(struct Computed_field *field,
	FE_value *values, int number_of_values, double time, struct FE_element **element, 
	FE_value *xi, int element_dimension, struct Cmiss_region *search_region,
	int propagate_field, int find_nearest_location);
/*******************************************************************************
LAST MODIFIED : 18 April 2005

DESCRIPTION :
This function implements the reverse of some certain computed_fields
(Computed_field_is_find_element_xi_capable) so that it tries to find an element
and xi which would evaluate to the given values.
This has been implemented so that the texture_coordinates can be used to extract
information from textures (sample_texture computed_field) and then modified and
then put back into another texture.
The <search_element_group> is the set of elements from which the chosen element
will belong or alternatively this can be NULL and the <*element> set to 
a single element to search in.
If <propagate_field> is set and the field has a find_element_xi_function, it
is called to undo its field calculation and resume the search on its source
field. This can result in less computation, but can fail if the source field
is multivalued, a common case being when it is in a polar coordinate system
since valid values may be a multiple of  2*PI out.
If <propagate_field> is not set or there is no <find_element_xi_function> this
function searches all elements in <search_element_group> trying to find a point
at which the field evaluates to the <values>.
If <propagate_field> is not set then <find_nearest_location> can be set and 
then rather than requiring an exact match the closest location in the 
<search_region> or the <*element> will be found.  If <propagate_field> is set
then the <find_nearest_location> flag is ignored.
Note a copy of the <values> array is immediately made so it will be possible to
pass in pointers to field cache values.
==============================================================================*/

int Computed_field_is_find_element_xi_capable(struct Computed_field *field,
	void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
This function returns true if the <field> can find element and xi given
a set of values.
==============================================================================*/

int define_Computed_field(struct Parse_state *state,void *field_copy_void,
	void *define_field_package_void);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Modifier entry function for creating and modifying Computed_fields. Format for
parameters from the parse state are:
  FIELD_NAME|NEW_FIELD_NAME
    components #
      rectangular_cartesian/cylindrical_polar/spherical_polar/prolate_sph...
        component
          FIELD_NAME.COMPONENT_NAME
        composite
				  scalars FIELD_NAME FIELD_NAME... FIELD_NAME{number_of_components}
        gradient
				  scalar FIELD_NAME
					coordinate FIELD_NAME
        rc_coordinate
					coordinate FIELD_NAME
        scale
				  field FIELD_NAME
				  values # # ... #{number_of_components}
        ... (more as more types added)
Note that the above layout is used because:
1. The number_of_components is often prerequisite information for setting up
the modifier functions for certain types of computed field, eg. "composite"
requires as many scalar fields as there are components, while scale has as many
floats.
2. The number_of_components and coordinate system are options for all types of
computed field so it makes sense that they are set before splitting up into the
options for the various types.
The <field_copy_void> parameter, if set, points to the field we are to modify
and should not itself be managed.
==============================================================================*/

int equivalent_computed_fields_at_elements(struct FE_element *element_1,
	struct FE_element *element_2);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Returns true if all fields are defined in the same way at the two elements.
==============================================================================*/

int equivalent_computed_fields_at_nodes(struct FE_node *node_1,
	struct FE_node *node_2);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Returns true if all fields are defined in the same way at the two nodes.
==============================================================================*/

struct Computed_field *Computed_field_manager_get_component_wrapper(
	struct MANAGER(Computed_field) *computed_field_manager,
	struct Computed_field *field,int component_no);
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
If a COMPONENT wrapper for <field> <component_no> exists in the
<computed_field_manager>, it is returned, otherwise a new one is made in the
manager and returned.
==============================================================================*/

int list_Computed_field(struct Computed_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Writes the properties of the <field> to the command window.
==============================================================================*/

int list_Computed_field_commands(struct Computed_field *field,
	void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Writes the properties of the <field> to the command window.
==============================================================================*/

int list_Computed_field_commands_if_managed_source_fields_in_list(
	struct Computed_field *field, void *list_commands_data_void);
/*******************************************************************************
LAST MODIFIED : 14 December 2001

DESCRIPTION :
Calls list_Computed_field_commands if the field is not already in the list,
has no source fields, or all its source fields are either not managed or
already in the list. If the field is listed, it is added to the list.
Ensures field command list comes out in the order they need to be created.
Note, must be cycled through as many times as it takes till listed_fields -> 0.
Second argument is a struct List_Computed_field_commands_data.
==============================================================================*/

int write_Computed_field_commands_to_comfile(struct Computed_field *field,
	 void *command_prefix_void);
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Writes the commands needed to reproduce <field> to the com file.
==============================================================================*/

int write_Computed_field_commands_if_managed_source_fields_in_list_to_comfile(
	 struct Computed_field *field, void *list_commands_data_void);
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Calls list_Computed_field_commands if the field is not already in the list,
has no source fields, or all its source fields are either not managed or
already in the list. If the field is listed, it is added to the list.
Ensures field command list comes out in the order they need to be created.
Note, must be cycled through as many times as it takes till listed_fields -> 0.
Second argument is a struct List_Computed_field_commands_data.
==============================================================================*/

int list_Computed_field_name(struct Computed_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 4 February 1999

DESCRIPTION :
Lists a single line about a computed field including just name, number of
components, coordinate system and type.
==============================================================================*/

struct Computed_field_package *CREATE(Computed_field_package)(
	struct MANAGER(Computed_field) *computed_field_manager);
/*******************************************************************************
LAST MODIFIED : 20 May 2008

DESCRIPTION :
Creates a Computed_field_package which is used by the rest of the program to
access everything to do with computed fields.
The root_region's computed_field_manager is passed in to support old code that
expects it to be in the package. This is temporary until all code gets the true
manager from the respective Cmiss_region.
==============================================================================*/

int DESTROY(Computed_field_package)(
	struct Computed_field_package **package_address);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Frees memory/deaccess objects in computed_field_package at <*package_address>.
Cancels any further messages from the root_region.
==============================================================================*/

struct MANAGER(Computed_field)
	*Computed_field_package_get_computed_field_manager(
		struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Extracts the computed_field_manager from the computed_field_package. Note that
the rest of the program should use this sparingly - it is really only here to
allow interfacing to the choose_object widgets.
==============================================================================*/

int Computed_field_package_remove_types(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Unregisters each of the computed field types added.
==============================================================================*/

int Computed_field_can_be_destroyed(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns true if the <field> is only accessed once - assumed by its manager. If
it is of type COMPUTED_FIELD_FINITE_ELEMENT further tests that its fe_field can
be destroyed, assuming it is only accessed by this field and its manager.
==============================================================================*/

/***************************************************************************//**
 * Returns the region which this field belongs to, if any.
 *
 * @param field  The field.
 * @return  A handle to the owning region if in one, or NULL if none.
 */
struct Cmiss_region *Computed_field_get_region(struct Computed_field *field);

/** 
 * Returns true if the field is a coordinate field, false otherwise
 *
 * @param field the field to determine whether or not it is a coordinate field
 * @return 1 if the field is a coordinate field, 0 otherwise
 */
int Computed_field_is_coordinate_field(struct Computed_field *field, void *not_in_use);

/*******************************************************************************
 * Returns the domains of the given field by recursively searching through the
 * field's source fields
 *
 * @param field  The field to find the domain of
 * @param domain_field_list  A handle to the list of domains for the field
 * @return  1 on success, 0 otherwise
 */
int Computed_field_get_domain( struct Computed_field *field,
	struct LIST(Computed_field) *domain_field_list );

/*******************************************************************************
 * Returns true if the field value varies non-linearly over its domain.
 * If applicable, e.g. for coordinate fields, callers should first determine
 * whether the coordinate system varies non-linearly with screen space since
 * that is a less expensive query.
 * Note that an image-based field is linear if it is interpolated linearly.
 *
 * @see Coordinate_system_type_is_non_linear
 * @param field  The field to query.
 * @return  1 if non-linear, 0 if linear.
 */
int Computed_field_is_non_linear(struct Computed_field *field);

/***************************************************************************//**
 * @return  Allocated field name unused by any other field in field_module.
 */
char *Cmiss_field_module_get_unique_field_name(
	struct Cmiss_field_module *field_module);

/***************************************************************************//**
 * Returns true if field is not a source field of other.
 * 
 * @param field  The field.
 * @return  1 if field is not a source field of others otherwise 0.
 */
int Computed_field_is_not_source_field_of_others(struct Computed_field *field);

#if defined (DEBUG_CODE)
/**
 * Returns the access count for the supplied field.  This function is only
 * available when DEBUG_CODE is defined.
 *
 * @param field the field to query the access count from
 * @return the access count
 */
int Cmiss_field_get_access_count(Cmiss_field_id field);
#endif /* defined (DEBUG_CODE) */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !defined (COMPUTED_FIELD_H) */
