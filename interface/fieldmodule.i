/**
 * fieldmodule.i
 *
 * Swig interface file for wrapping api functions in api/fieldmodule.hpp
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") fieldmodule

%extend OpenCMISS::Zinc::Fieldmodulenotifier {

    int setCallback(PyObject *callbackObject)
    {
        PyObject *my_callback = NULL;
        if (!PyCallable_Check(callbackObject))
        {
            PyErr_SetString(PyExc_TypeError, "callbackObject must be callable");
            return 0;
        }
        Py_XINCREF(callbackObject);         /* Add a reference to new callback */     /* Remember new callback */
        return cmzn_fieldmodulenotifier_set_callback(($self)->getId(), fieldmoduleCallbackToPython, (void *)callbackObject);
    }

    int clearCallback()
    {
      	void *user_data = cmzn_fieldmodulenotifier_get_callback_user_data(($self)->getId());
	    PyObject *callbackObject = static_cast<PyObject *>(user_data);
	    Py_XDECREF(callbackObject);         /* Decrease a reference count */
        return cmzn_fieldmodulenotifier_clear_callback(($self)->getId());
    }
}

%ignore OpenCMISS::Zinc::Fieldmodulenotifier::clearCallback();

%include "doublevaluesarraytypemap.i"
%include "integervaluesarraytypemap.i"
%include "fieldarraytypemap.i"
%include "pyzincstringhandling.i"

%import "scenecoordinatesystem.i"
%import "timesequence.i"
%import "optimisation.i"
%import "field.i"
%import "fieldcache.i"

%{
#include "zinc/fieldalias.hpp"
#include "zinc/fieldarithmeticoperators.hpp"
#include "zinc/fieldcache.hpp"
#include "zinc/fieldcomposite.hpp"
#include "zinc/fieldconditional.hpp"
#include "zinc/fieldconstant.hpp"
#include "zinc/fieldcoordinatetransformation.hpp"
#include "zinc/fieldderivatives.hpp"
#include "zinc/fieldfibres.hpp"
#include "zinc/fieldfiniteelement.hpp"
#include "zinc/fieldsubobjectgroup.hpp"
#include "zinc/fieldgroup.hpp"
#include "zinc/fieldimage.hpp"
#include "zinc/fieldimageprocessing.hpp"
#include "zinc/fieldlogicaloperators.hpp"
#include "zinc/fieldmatrixoperators.hpp"
#include "zinc/fieldmeshoperators.hpp"
#include "zinc/fieldnodesetoperators.hpp"
#include "zinc/fieldsceneviewerprojection.hpp"
#include "zinc/fieldtime.hpp"
#include "zinc/fieldtrigonometry.hpp"
#include "zinc/fieldvectoroperators.hpp"
#include "zinc/fieldmodule.hpp"
#include "zinc/optimisation.hpp"

static void fieldmoduleCallbackToPython(cmzn_fieldmoduleevent_id fieldmoduleevent, void *user_data)
{
    PyObject *arglist = NULL;
    PyObject *result = NULL;
    PyObject *my_callback = (PyObject *)user_data;
    /* convert fieldmoduleevent to python object */
    PyObject *obj = NULL;
    OpenCMISS::Zinc::Fieldmoduleevent *fieldmoduleEvent = new OpenCMISS::Zinc::Fieldmoduleevent(cmzn_fieldmoduleevent_access(fieldmoduleevent));
    obj = SWIG_NewPointerObj(SWIG_as_voidptr(fieldmoduleEvent), SWIGTYPE_p_OpenCMISS__Zinc__Fieldmoduleevent, 1);
    /* Time to call the callback */
    arglist = Py_BuildValue("(N)", obj);
    result = PyObject_CallObject(my_callback, arglist);
    Py_DECREF(arglist);
    if (result)
    {
        Py_DECREF(result);
    }
}
%}

%include "zinc/fieldmodule.hpp"

