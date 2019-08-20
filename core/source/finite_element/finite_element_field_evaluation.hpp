/**
 * FILE : finite_element_field_evaluation.hpp
 *
 * Class for caching values and evaluating fields over elements.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_FIELD_EVALUATION_HPP)
#define FINITE_ELEMENT_FIELD_EVALUATION_HPP

#include <opencmiss/zinc/zincconfigure.h>
#include "opencmiss/zinc/types/elementid.h"
#include "finite_element/finite_element_basis.h"
#include "finite_element/finite_element_field.hpp"
#include "general/value.h"

/*
Global types
------------
*/

/**
 * Values and methods for interpolating a field over a single element.
 * Caches values mapped from nodes or other parameter storage in efficient
 * form for fast interpolation with basis, allowing derivative evaluation etc.
 * Expensive to compute the first time, but multiple evaluations in the
 * same element should result in savings in overall compute time.
 * Note for time-varying fields: values are valid at a single time only.
 * Client is responsible for recomputing if field parameters change, or
 * evaluating at a different element, time etc.
 */
class FE_element_field_evaluation
{
	/* the FE_field these values are for */
	struct FE_field *field;
	/* the element these values are for */
	cmzn_element *element;
	/* the element the field was inherited from */
	cmzn_element *field_element;
	/* whether or not these values depend on time */
	int time_dependent;
	/* if the values are time dependent, the time at which they were calculated */
	FE_value time;
	/* number of sub-elements in each xi-direction of element for each component.
	 * If NULL (for a component) then field is not grid based for that component.
		 Notes:
		1.  Grid sub-elements are linear in each direction.  This means that
			<component_number_of_values> is not used
		2.  the grid-point values are not blended (to monomial) and so
			<component_standard_basis_functions> and
			<component_standard_basis_function_arguments> are not used
		3.  for grid-based <destroy_standard_basis_arguments> is used to specify
			if the <component_values> should be destroyed (element field has been
			inherited) */
	int **component_number_in_xi;
	/* a flag to specify whether or not values have also been calculated for the
		derivatives of the field with respect to the xi coordinates */
	bool derivatives_calculated;
	/* a flag added to specify if the element field component modify function is
		ignored */
	/* specify whether the standard basis arguments should be destroyed (element
		field has been inherited) or not be destroyed (element field is defined for
		the element and the basis arguments are being used) */
	bool destroy_standard_basis_arguments;
	/* the number of field components */
	int number_of_components;
	/* the number of values for each component */
	int *component_number_of_values;
	/* the values_storage for each component if grid-based */
	const Value_storage **component_grid_values_storage;
	/* grid_offset_in_xi is allocated with 2^number_of_xi_coordinates integers
		 giving the increment in index into the values stored with the top_level
		 element for the grid. For top_level_elements the first value is 1, the
		 second is (number_in_xi[0]+1), the third is
		 (number_in_xi[0]+1)*(number_in_xi[1]+1) etc. The base_grid_offset is 0 for
		 top_level_elements. For faces and lines these values are adjusted to get
		 the correct index for the top_level_element */
	int *component_base_grid_offset,**component_grid_offset_in_xi;
	/* following allocated with 2^number_of_xi for grid-based fields for use in
		 calculate_FE_element_field */
	int *element_value_offsets;
	/* the values for each component */
	FE_value **component_values;
	/* the standard basis function for each component */
	Standard_basis_function **component_standard_basis_functions;
	/* the arguments for the standard basis function for each component */
	int **component_standard_basis_function_arguments;
	/* working space for evaluating basis, for grid-based only */
	FE_value *grid_basis_function_values;
	int access_count;

	FE_element_field_evaluation();

	~FE_element_field_evaluation()
	{
		this->clear();
	}

public:
	/** create on heap with access_count = 1 */
	static FE_element_field_evaluation *create()
	{
		return new FE_element_field_evaluation();
	}

	inline FE_element_field_evaluation *access()
	{
		++access_count;
		return this;
	}

	static int deaccess(FE_element_field_evaluation* &element_field_evaluation);

	/** Free the memory for the fields of the evaluation object.
	 * Restores it to the blank state it was created with.
	 * Must be called before calling calculate_values() again. */
	void clear();

	/** Determine if existing values are valid to use for element and time.
	 * @param time_in  If values are time dependent, match against this time.
	 * @param field_element_in  Optional field element to inherit from; 0 to ignore
	 */
	bool is_for_element_and_time(cmzn_element *element_in, FE_value time_in, cmzn_element *field_element_in) const
	{
		return (element_in == this->element)
			&& ((!field_element_in) || (field_element_in = this->field_element))
			&& ((!this->time_dependent) || (time_in == this->time));
	}

	bool has_derivatives_calculated() const
	{
		return this->derivatives_calculated;
	}

	/** Fill the evaluation structure, ready to evaluate field.
	 * Must be freshly created or cleared.
	 * @param topLevelElement  Optional element to inherit field from.
	 */
	int calculate_values(struct FE_field *field, cmzn_element *element,
		FE_value time, bool calculate_derivatives, cmzn_element *top_level_element);

	/** Modifies the calculated values so that it will calculate
	 * derivatives wrt xi_index for the original field. 
	 * Currently only implemented for monomials. */
	int differentiate(int xi_index);

	/** Evaluate integer field values in element.
	 * @param component_number  Component number to evaluate starting at 0, or any
	 * other value to evaluate all components.
	 * @param xi_coordinates  Element chart location to evaluate at.
	 * @param values  Caller-supplied space to store the int values. */
	int evaluate_int(int component_number, const FE_value *xi_coordinates,
		int *values);

	/** Evaluate real field/component values in element. Converts other numerical
	 * types to real. Must have called calculate_values first, with derivatives
	 * if jacobian requested.
	 * @param component_number  Component number to evaluate starting at 0, or any
	 * other value to evaluate all components.
	 * @param xi_coordinates  Element chart location to evaluate at.
	 * @param basis_function_values.  Standard basis function evaluation cache.
	 * @param values  Caller-supplied space to store the real values.
	 * @param jacobian  If non-zero, points to memory to receive N*M values,
	 * of N component first derivatives with respect to M xi coordinates. */
	int evaluate_real(int component_number, const FE_value *xi_coordinates,
		Standard_basis_function_values &basis_function_values, FE_value *values, FE_value *jacobian);

	/** Returns allocated copies of the string values of the field in the element.
	 * @param component_number  Component number to evaluate starting at 0, or any
	 * other value to evaluate all components.
	 * @param xi_coordinates  Element chart location to evaluate at.
	 * @param values  Caller-supplied space to store the allocated string/s.
	 * It is up to the caller to deallocate the returned strings. */
	int evaluate_string(int component_number, const FE_value *xi_coordinates,
		char **values);

	/** Calculate the values of element field in element, convert to and
	 * return as an allocated string. Derivatives are not included in string.
	 * @param component_number  Component number to evaluate starting at 0, or any
	 * other value to evaluate all components.
	 * @param xi_coordinates  Element chart location to evaluate at.
	 * @param basis_function_values.  Standard basis function evaluation cache.
	 * @param out_string  Pointer to address to store allocated string pointer.
	 * It is up to the caller to deallocate the returned string. */
	int evaluate_as_string(int component_number, const FE_value *xi_coordinates,
		Standard_basis_function_values &basis_function_values, char **out_string);

	/** Allocates and returns component count and values for component_number.
	 * It is up to the caller to deallocate any returned component values. */
	int get_component_values(int component_number,
		int *number_of_component_values_address, FE_value **component_values_address) const;

	/** If component_number is monomial, integer values describing the monomial
	 * basis are returned. The first number is the dimension, the following
	 * numbers are the order of the monomial in each direction, e.g. 3=cubic.
	 * @param monomial_info  Must point to a block of memory big enough to take
	 * 1 + MAXIMUM_ELEMENT_XI_DIMENSIONS integers. */
	int get_monomial_component_info(int component_number, int *monomial_info) const;

};

#endif /* !defined (FINITE_ELEMENT_FIELD_EVALUATION_HPP) */
