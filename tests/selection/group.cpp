/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/element.h>
#include <cmlibs/zinc/field.h>
#include <cmlibs/zinc/fieldcache.h>
#include <cmlibs/zinc/fieldcomposite.h>
#include <cmlibs/zinc/fieldconstant.h>
#include <cmlibs/zinc/fieldfiniteelement.h>
#include <cmlibs/zinc/fieldgroup.h>
#include <cmlibs/zinc/fieldlogicaloperators.h>
#include <cmlibs/zinc/fieldmeshoperators.hpp>
#include <cmlibs/zinc/fieldnodesetoperators.hpp>
#include <cmlibs/zinc/fieldsubobjectgroup.h>
#include <cmlibs/zinc/node.h>
#include <cmlibs/zinc/region.h>

#include <cmlibs/zinc/element.hpp>
#include <cmlibs/zinc/field.hpp>
#include <cmlibs/zinc/fieldcache.hpp>
#include <cmlibs/zinc/fieldcomposite.hpp>
#include <cmlibs/zinc/fieldconstant.hpp>
#include <cmlibs/zinc/fieldfiniteelement.hpp>
#include <cmlibs/zinc/fieldgroup.hpp>
#include <cmlibs/zinc/fieldlogicaloperators.hpp>
#include <cmlibs/zinc/fieldsubobjectgroup.hpp>
#include <cmlibs/zinc/node.hpp>
#include <cmlibs/zinc/region.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

void addSomeNodes(cmzn_fieldmodule_id fm)
{
	cmzn_fieldmodule_begin_change(fm);
	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset_id>(0), nodeset);
	cmzn_nodetemplate_id nodetemplate = cmzn_nodeset_create_nodetemplate(nodeset);
	EXPECT_NE(static_cast<cmzn_nodetemplate_id>(0), nodetemplate);
	int result;
	for (int i = 1; i <= 4; ++i)
	{
		cmzn_node_id node = cmzn_nodeset_create_node(nodeset, -1, nodetemplate);
		EXPECT_NE(static_cast<cmzn_node_id>(0), node);
		EXPECT_EQ(CMZN_OK, result = cmzn_node_destroy(&node));
	}
	cmzn_nodetemplate_destroy(&nodetemplate);
	cmzn_nodeset_destroy(&nodeset);
	cmzn_fieldmodule_end_change(fm);
}

void addSomeElements(cmzn_fieldmodule_id fm)
{
	cmzn_fieldmodule_begin_change(fm);
	cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(fm, 3);
	EXPECT_NE(static_cast<cmzn_mesh_id>(0), mesh);
	cmzn_elementtemplate_id elementtemplate = cmzn_mesh_create_elementtemplate(mesh);
	EXPECT_NE(static_cast<cmzn_elementtemplate_id>(0), elementtemplate);
	EXPECT_EQ(CMZN_OK, cmzn_elementtemplate_set_element_shape_type(elementtemplate, CMZN_ELEMENT_SHAPE_TYPE_CUBE));
	for (int i = 1; i <= 4; ++i)
		EXPECT_EQ(CMZN_OK, cmzn_mesh_define_element(mesh, -1, elementtemplate));
	cmzn_elementtemplate_destroy(&elementtemplate);
	cmzn_mesh_destroy(&mesh);
	cmzn_fieldmodule_end_change(fm);
}

TEST(cmzn_fieldgroup, add_remove_nodes)
{
	ZincTestSetup zinc;
	int result;

	cmzn_region_id childRegion = cmzn_region_create_child(zinc.root_region, "child");
	cmzn_fieldmodule_id childFm = cmzn_region_get_fieldmodule(childRegion);
	addSomeNodes(childFm);
	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(childFm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	cmzn_node_id node = cmzn_nodeset_find_node_by_identifier(nodeset, 2);
	EXPECT_NE(static_cast<cmzn_node_id>(0), node);

	cmzn_field_id groupField = cmzn_fieldmodule_create_field_group(zinc.fm);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(groupField, "group"));
	cmzn_field_group_id group = cmzn_field_cast_group(groupField);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);

	// test find/create node group for a subregion
	cmzn_field_node_group_id nodeGroup = 0;
	EXPECT_EQ(static_cast<cmzn_field_node_group_id>(0), nodeGroup = cmzn_field_group_get_field_node_group(group, nodeset));
	EXPECT_NE(static_cast<cmzn_field_node_group_id>(0), nodeGroup = cmzn_field_group_create_field_node_group(group, nodeset));
	EXPECT_EQ(static_cast<cmzn_field_node_group_id>(0), cmzn_field_group_create_field_node_group(group, nodeset));
	char *childNodeGroupName = cmzn_field_get_name(cmzn_field_node_group_base_cast(nodeGroup));
	EXPECT_STREQ("group.nodes", childNodeGroupName);
	cmzn_deallocate(childNodeGroupName);

	// test contains/add
	cmzn_nodeset_group_id nodesetGroup = cmzn_field_node_group_get_nodeset_group(nodeGroup);
	EXPECT_NE(static_cast<cmzn_nodeset_group_id>(0), nodesetGroup);
	EXPECT_FALSE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node));
	EXPECT_EQ(CMZN_OK, cmzn_nodeset_group_add_node(nodesetGroup, node));
	EXPECT_TRUE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node));

	// test child group created properly, and that we find non-empty subgroup
	cmzn_field_group_id childGroup = 0;
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));
	char *childGroupName = cmzn_field_get_name(cmzn_field_group_base_cast(childGroup));
	EXPECT_STREQ("group", childGroupName);
	cmzn_deallocate(childGroupName);
	cmzn_field_group_id nonEmptyGroup = 0;
	EXPECT_EQ(childGroup, nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	cmzn_field_group_destroy(&nonEmptyGroup);
	cmzn_field_node_group_id tmpNodeGroup = 0;
	EXPECT_EQ(nodeGroup, tmpNodeGroup = cmzn_field_group_get_field_node_group(childGroup, nodeset));
	cmzn_field_node_group_destroy(&tmpNodeGroup);
	cmzn_field_group_destroy(&childGroup);

	// test remove/contains
	EXPECT_EQ(CMZN_OK, cmzn_nodeset_group_remove_node(nodesetGroup, node));
	EXPECT_FALSE(cmzn_nodeset_contains_node(cmzn_nodeset_group_base_cast(nodesetGroup), node));

	// test for empty groups
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	EXPECT_EQ(CMZN_OK, cmzn_field_group_remove_empty_subgroups(group));
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));

	cmzn_nodeset_group_destroy(&nodesetGroup);
	cmzn_field_node_group_destroy(&nodeGroup);
	cmzn_field_group_destroy(&group);
	cmzn_field_destroy(&groupField);

	cmzn_node_destroy(&node);
	cmzn_nodeset_destroy(&nodeset);
	cmzn_fieldmodule_destroy(&childFm);
	cmzn_region_destroy(&childRegion);
}

TEST(ZincFieldGroup, add_remove_nodes)
{
	ZincTestSetupCpp zinc;
	int result;

	Region childRegion = zinc.root_region.createChild("child");
	Fieldmodule childFm = childRegion.getFieldmodule();
	addSomeNodes(childFm.getId());
	Nodeset nodeset = childFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	Node node = nodeset.findNodeByIdentifier(2);
	EXPECT_TRUE(node.isValid());

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(CMZN_OK, result = group.setName("group"));

	// test find/create node group for a subregion
	FieldNodeGroup nodeGroup;
	EXPECT_FALSE((nodeGroup = group.getFieldNodeGroup(nodeset)).isValid());
	EXPECT_TRUE((nodeGroup = group.createFieldNodeGroup(nodeset)).isValid());
	EXPECT_FALSE(group.createFieldNodeGroup(nodeset).isValid());
	char *childNodeGroupName = nodeGroup.getName();
	EXPECT_STREQ("group.nodes", childNodeGroupName);
	cmzn_deallocate(childNodeGroupName);

	// test contains/add
	NodesetGroup nodesetGroup = nodeGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());
	EXPECT_FALSE(nodesetGroup.containsNode(node));
	EXPECT_EQ(OK, nodesetGroup.addNode(node));
	EXPECT_TRUE(nodesetGroup.containsNode(node));

	// test child group created properly, and that we find non-empty subgroup
	FieldGroup childGroup;
	EXPECT_TRUE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
	char *childGroupName = childGroup.getName();
	EXPECT_STREQ("group", childGroupName);
	cmzn_deallocate(childGroupName);
	FieldGroup nonEmptyGroup;
	EXPECT_EQ(childGroup, nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup());
	FieldNodeGroup tmpNodeGroup;
	EXPECT_EQ(nodeGroup, tmpNodeGroup = childGroup.getFieldNodeGroup(nodeset));

	// test remove/contains
	EXPECT_EQ(OK, nodesetGroup.removeNode(node));
	EXPECT_FALSE(nodesetGroup.containsNode(node));

	// test for empty groups
	childGroup = FieldGroup();  // so not rediscovered by name
	EXPECT_FALSE((nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup()).isValid());
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_FALSE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
}

TEST(cmzn_fieldgroup, add_remove_elements)
{
	ZincTestSetup zinc;
	int result;

	cmzn_region_id childRegion = cmzn_region_create_child(zinc.root_region, "child");
	cmzn_fieldmodule_id childFm = cmzn_region_get_fieldmodule(childRegion);
	addSomeElements(childFm);
	cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(childFm, 3);
	cmzn_element_id element = cmzn_mesh_find_element_by_identifier(mesh, 2);
	EXPECT_NE(static_cast<cmzn_element_id>(0), element);

	cmzn_field_id groupField = cmzn_fieldmodule_create_field_group(zinc.fm);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_set_name(groupField, "group"));
	cmzn_field_group_id group = cmzn_field_cast_group(groupField);
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), group);

	// test find/create element group for a subregion
	cmzn_field_element_group_id elementGroup = 0;
	EXPECT_EQ(static_cast<cmzn_field_element_group_id>(0), elementGroup = cmzn_field_group_get_field_element_group(group, mesh));
	EXPECT_NE(static_cast<cmzn_field_element_group_id>(0), elementGroup = cmzn_field_group_create_field_element_group(group, mesh));
	EXPECT_EQ(static_cast<cmzn_field_element_group_id>(0), cmzn_field_group_create_field_element_group(group, mesh));
	char *childNodeGroupName = cmzn_field_get_name(cmzn_field_element_group_base_cast(elementGroup));
	EXPECT_STREQ("group.mesh3d", childNodeGroupName);
	cmzn_deallocate(childNodeGroupName);

	// test contains/add
	cmzn_mesh_group_id meshGroup = cmzn_field_element_group_get_mesh_group(elementGroup);
	EXPECT_NE(static_cast<cmzn_mesh_group_id>(0), meshGroup);
	EXPECT_FALSE(cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), element));
	EXPECT_EQ(CMZN_OK, cmzn_mesh_group_add_element(meshGroup, element));
	EXPECT_TRUE(cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), element));

	// test child group created properly, and that we find non-empty subgroup
	cmzn_field_group_id childGroup = 0;
	EXPECT_NE(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));
	char *childGroupName = cmzn_field_get_name(cmzn_field_group_base_cast(childGroup));
	EXPECT_STREQ("group", childGroupName);
	cmzn_deallocate(childGroupName);
	cmzn_field_group_id nonEmptyGroup = 0;
	EXPECT_EQ(childGroup, nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	cmzn_field_group_destroy(&nonEmptyGroup);
	cmzn_field_element_group_id tmpNodeGroup = 0;
	EXPECT_EQ(elementGroup, tmpNodeGroup = cmzn_field_group_get_field_element_group(childGroup, mesh));
	cmzn_field_element_group_destroy(&tmpNodeGroup);
	cmzn_field_group_destroy(&childGroup);

	// test remove/contains
	EXPECT_EQ(CMZN_OK, cmzn_mesh_group_remove_element(meshGroup, element));
	EXPECT_FALSE(cmzn_mesh_contains_element(cmzn_mesh_group_base_cast(meshGroup), element));

	// test for empty groups
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), nonEmptyGroup = cmzn_field_group_get_first_non_empty_subregion_field_group(group));
	EXPECT_EQ(CMZN_OK, cmzn_field_group_remove_empty_subgroups(group));
	EXPECT_EQ(static_cast<cmzn_field_group_id>(0), childGroup = cmzn_field_group_get_subregion_field_group(group, childRegion));

	cmzn_mesh_group_destroy(&meshGroup);
	cmzn_field_element_group_destroy(&elementGroup);
	cmzn_field_group_destroy(&group);
	cmzn_field_destroy(&groupField);

	cmzn_element_destroy(&element);
	cmzn_mesh_destroy(&mesh);
	cmzn_fieldmodule_destroy(&childFm);
	cmzn_region_destroy(&childRegion);
}

TEST(ZincFieldGroup, add_remove_elements)
{
	ZincTestSetupCpp zinc;
	int result;

	Region childRegion = zinc.root_region.createChild("child");
	Fieldmodule childFm = childRegion.getFieldmodule();
	addSomeElements(childFm.getId());
	Mesh mesh = childFm.findMeshByDimension(3);
	EXPECT_TRUE(mesh.isValid());
	Element element = mesh.findElementByIdentifier(2);
	EXPECT_TRUE(element.isValid());

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(CMZN_OK, result = group.setName("group"));

	// test find/create element group for a subregion
	FieldElementGroup elementGroup;
	EXPECT_FALSE((elementGroup = group.getFieldElementGroup(mesh)).isValid());
	EXPECT_TRUE((elementGroup = group.createFieldElementGroup(mesh)).isValid());
	EXPECT_FALSE(group.createFieldElementGroup(mesh).isValid());
	char *childElementGroupName = elementGroup.getName();
	EXPECT_STREQ("group.mesh3d", childElementGroupName);
	cmzn_deallocate(childElementGroupName);

	// test contains/add
	MeshGroup meshGroup = elementGroup.getMeshGroup();
	EXPECT_TRUE(meshGroup.isValid());
	EXPECT_FALSE(meshGroup.containsElement(element));
	EXPECT_EQ(OK, meshGroup.addElement(element));
	EXPECT_TRUE(meshGroup.containsElement(element));

	// test child group created properly, and that we find non-empty subgroup
	FieldGroup childGroup;
	EXPECT_TRUE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
	char *childGroupName = childGroup.getName();
	EXPECT_STREQ("group", childGroupName);
	cmzn_deallocate(childGroupName);
	FieldGroup nonEmptyGroup;
	EXPECT_EQ(childGroup, nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup());
	FieldElementGroup tmpElementGroup;
	EXPECT_EQ(elementGroup, tmpElementGroup = childGroup.getFieldElementGroup(mesh));

	// test remove/contains
	EXPECT_EQ(OK, meshGroup.removeElement(element));
	EXPECT_FALSE(meshGroup.containsElement(element));

	// test for empty groups
	childGroup = FieldGroup();  // so not rediscovered by name
	EXPECT_FALSE((nonEmptyGroup = group.getFirstNonEmptySubregionFieldGroup()).isValid());
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_FALSE((childGroup = group.getSubregionFieldGroup(childRegion)).isValid());
}

TEST(ZincFieldElementGroup, add_remove_conditional)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/two_cubes.exformat").c_str()));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());

	int size1, size2, size3;
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(2, size3 = mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(11, size2 = mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(20, size1 = mesh1d.getSize());

	FieldElementGroup facesGroup = group.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(facesGroup.isValid());
	MeshGroup facesMeshGroup = facesGroup.getMeshGroup();
	EXPECT_TRUE(facesMeshGroup.isValid());
	EXPECT_EQ(0, facesMeshGroup.getSize());

	EXPECT_EQ(ERROR_ARGUMENT, facesMeshGroup.addElementsConditional(Field()));
	EXPECT_EQ(ERROR_ARGUMENT, facesMeshGroup.removeElementsConditional(Field()));

	FieldElementGroup linesGroup = group.createFieldElementGroup(mesh1d);
	EXPECT_TRUE(linesGroup.isValid());
	MeshGroup linesMeshGroup = linesGroup.getMeshGroup();
	EXPECT_TRUE(linesMeshGroup.isValid());
	EXPECT_EQ(0, linesMeshGroup.getSize());

	FieldIsOnFace isOnFaceField = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_XI1_0);
	EXPECT_TRUE(isOnFaceField.isValid());

	const double oneValue = 1.0;
	FieldConstant trueField = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(trueField.isValid());

	Element element1 = mesh2d.findElementByIdentifier(1);
	Element element2 = mesh2d.findElementByIdentifier(2);

	EXPECT_EQ(OK, facesMeshGroup.addElementsConditional(isOnFaceField));
	EXPECT_EQ(2, result = facesMeshGroup.getSize());
	EXPECT_TRUE(facesMeshGroup.containsElement(element1));
	EXPECT_TRUE(facesMeshGroup.containsElement(element2));

	EXPECT_EQ(0, linesMeshGroup.getSize());

	// copy into another element group to test conditional add/removal
	// optimisation for smaller groups
	FieldElementGroup otherFacesGroup = zinc.fm.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(otherFacesGroup.isValid());
	MeshGroup otherFacesMeshGroup = otherFacesGroup.getMeshGroup();
	EXPECT_TRUE(otherFacesMeshGroup.isValid());
	EXPECT_EQ(OK, otherFacesMeshGroup.addElementsConditional(facesGroup));
	EXPECT_EQ(2, result = otherFacesMeshGroup.getSize());
	EXPECT_TRUE(otherFacesMeshGroup.containsElement(element1));
	EXPECT_TRUE(otherFacesMeshGroup.containsElement(element2));

	// add all faces for conditional removal
	EXPECT_EQ(OK, facesMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(11, result = facesMeshGroup.getSize());

	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(isOnFaceField));
	EXPECT_EQ(9, result = facesMeshGroup.getSize());
	EXPECT_FALSE(facesMeshGroup.containsElement(element1));
	EXPECT_FALSE(facesMeshGroup.containsElement(element2));

	// test using self as a conditional
	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(facesGroup));
	EXPECT_EQ(0, facesMeshGroup.getSize());

	EXPECT_EQ(OK, facesMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(11, result = facesMeshGroup.getSize());

	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(otherFacesGroup));
	EXPECT_EQ(9, result = facesMeshGroup.getSize());
	EXPECT_FALSE(facesMeshGroup.containsElement(element1));
	EXPECT_FALSE(facesMeshGroup.containsElement(element2));

	// check not an error to remove nodes already removed
	EXPECT_EQ(OK, facesMeshGroup.removeAllElements());
	EXPECT_EQ(0, result = facesMeshGroup.getSize());
	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(otherFacesGroup));
}

TEST(ZincFieldNodeGroup, add_remove_conditional)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/two_cubes.exformat").c_str()));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());

	int size;
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(12, size = nodeset.getSize());

	FieldNodeGroup nodesGroup = group.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(nodesGroup.isValid());
	NodesetGroup nodesetGroup = nodesGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());
	EXPECT_EQ(0, nodesetGroup.getSize());

	EXPECT_EQ(ERROR_ARGUMENT, nodesetGroup.addNodesConditional(Field()));
	EXPECT_EQ(ERROR_ARGUMENT, nodesetGroup.removeNodesConditional(Field()));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	FieldComponent x = zinc.fm.createFieldComponent(coordinates, 1);
	EXPECT_TRUE(x.isValid());
	const double fifteen = 15.0;
	FieldGreaterThan x_gt_15 = x > zinc.fm.createFieldConstant(1, &fifteen);
	EXPECT_TRUE(x_gt_15.isValid());

	const double oneValue = 1.0;
	FieldConstant trueField = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(trueField.isValid());

	EXPECT_EQ(OK, nodesetGroup.addNodesConditional(x_gt_15));
	EXPECT_EQ(4, result = nodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_TRUE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// copy into another element group to test conditional add/removal
	// optimisation for smaller groups
	FieldNodeGroup otherNodesGroup = zinc.fm.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(otherNodesGroup.isValid());
	NodesetGroup otherNodesetGroup = otherNodesGroup.getNodesetGroup();
	EXPECT_TRUE(otherNodesetGroup.isValid());
	EXPECT_EQ(OK, otherNodesetGroup.addNodesConditional(nodesGroup));
	EXPECT_EQ(4, result = otherNodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_TRUE(otherNodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// add all faces for conditional removal
	EXPECT_EQ(OK, nodesetGroup.addNodesConditional(trueField));
	EXPECT_EQ(12, result = nodesetGroup.getSize());

	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(x_gt_15));
	EXPECT_EQ(8, result = nodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_FALSE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// test using self as a conditional
	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(nodesGroup));
	EXPECT_EQ(0, nodesetGroup.getSize());

	EXPECT_EQ(OK, nodesetGroup.addNodesConditional(trueField));
	EXPECT_EQ(12, result = nodesetGroup.getSize());

	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(otherNodesGroup));
	EXPECT_EQ(8, result = nodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_FALSE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// check not an error to remove nodes already removed
	EXPECT_EQ(OK, nodesetGroup.removeAllNodes());
	EXPECT_EQ(0, result = nodesetGroup.getSize());
	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(otherNodesGroup));
}

TEST(ZincFieldGroup, subelementHandlingMode)
{
	ZincTestSetupCpp zinc;
	int result;

	Region childRegion = zinc.root_region.createChild("child");
	EXPECT_TRUE(childRegion.isValid());

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());

	FieldGroup::SubelementHandlingMode mode;
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE, mode = group.getSubelementHandlingMode());
	EXPECT_EQ(ERROR_ARGUMENT, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_INVALID));
	EXPECT_EQ(OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL, mode = group.getSubelementHandlingMode());

	// check subelement handling mode is inherited by new subregion groups
	FieldGroup childGroup = group.createSubregionFieldGroup(childRegion);
	EXPECT_TRUE(childGroup.isValid());
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL, mode = childGroup.getSubelementHandlingMode());

	EXPECT_EQ(OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE));
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE, mode = childGroup.getSubelementHandlingMode());

	// orphan child group, set new handling mode, rediscover child group and check mode
	EXPECT_EQ(OK, result = group.clear());
	EXPECT_EQ(OK, result = group.removeEmptySubgroups());
	EXPECT_EQ(OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL, mode = group.getSubelementHandlingMode());
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE, mode = childGroup.getSubelementHandlingMode());

	// subregion group can now be rediscovered by name using getSubregionFieldGroup()
	FieldGroup tempGroup = group.getSubregionFieldGroup(childRegion);
	EXPECT_TRUE(tempGroup.isValid());
	EXPECT_EQ(childGroup, tempGroup);
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_NONE, mode = childGroup.getSubelementHandlingMode());
}

TEST(ZincFieldElementGroup, add_remove_with_subelement_handling)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/two_cubes.exformat").c_str()));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(OK, result = group.setName("group"));
	FieldGroup::SubelementHandlingMode mode;
	EXPECT_EQ(OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	EXPECT_EQ(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL, mode = group.getSubelementHandlingMode());

	int size0, size1, size2, size3;
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(2, size3 = mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(11, size2 = mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(20, size1 = mesh1d.getSize());
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(12, size0 = nodeset.getSize());

	Element element1 = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	Element element2 = mesh3d.findElementByIdentifier(2);
	EXPECT_TRUE(element2.isValid());

	FieldElementGroup elementsGroup = group.createFieldElementGroup(mesh3d);
	EXPECT_TRUE(elementsGroup.isValid());
	MeshGroup elementsMeshGroup = elementsGroup.getMeshGroup();
	EXPECT_TRUE(elementsMeshGroup.isValid());
	EXPECT_EQ(0, elementsMeshGroup.getSize());

	FieldElementGroup facesGroup = group.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(facesGroup.isValid());
	MeshGroup facesMeshGroup = facesGroup.getMeshGroup();
	EXPECT_TRUE(facesMeshGroup.isValid());

	FieldElementGroup linesGroup = group.createFieldElementGroup(mesh1d);
	EXPECT_TRUE(linesGroup.isValid());
	MeshGroup linesMeshGroup = linesGroup.getMeshGroup();
	EXPECT_TRUE(linesMeshGroup.isValid());

	FieldNodeGroup nodesGroup = group.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(nodesGroup.isValid());
	NodesetGroup nodesetGroup = nodesGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());

	EXPECT_EQ(0, elementsMeshGroup.getSize());
	EXPECT_EQ(0, facesMeshGroup.getSize());
	EXPECT_EQ(0, linesMeshGroup.getSize());
	EXPECT_EQ(0, nodesetGroup.getSize());

	const double oneValue = 1.0;
	FieldConstant trueField = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(trueField.isValid());

	EXPECT_EQ(OK, result = elementsMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(2, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(11, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(20, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(12, size0 = nodesetGroup.getSize());

	EXPECT_EQ(OK, result = elementsMeshGroup.removeAllElements());
	EXPECT_EQ(0, result = elementsMeshGroup.getSize());
	EXPECT_EQ(0, result = facesMeshGroup.getSize());
	EXPECT_EQ(0, result = linesMeshGroup.getSize());
	EXPECT_EQ(0, result = nodesetGroup.getSize());

	EXPECT_EQ(OK, result = elementsMeshGroup.addElement(element1));
	EXPECT_EQ(1, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(6, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(12, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(8, size0 = nodesetGroup.getSize());
	EXPECT_EQ(OK, result = elementsMeshGroup.addElement(element2));
	EXPECT_EQ(2, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(11, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(20, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(12, size0 = nodesetGroup.getSize());

	EXPECT_EQ(OK, result = elementsMeshGroup.removeElement(element1));
	EXPECT_EQ(1, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(6, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(12, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(8, size0 = nodesetGroup.getSize());
	EXPECT_EQ(OK, result = elementsMeshGroup.removeElement(element2));
	EXPECT_EQ(0, result = elementsMeshGroup.getSize());
	EXPECT_EQ(0, result = facesMeshGroup.getSize());
	EXPECT_EQ(0, result = linesMeshGroup.getSize());
	EXPECT_EQ(0, result = nodesetGroup.getSize());

	FieldIsOnFace isOnFaceField = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_XI2_0);
	EXPECT_TRUE(isOnFaceField.isValid());

	EXPECT_EQ(OK, result = facesMeshGroup.addElementsConditional(isOnFaceField));
	EXPECT_EQ(0, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(2, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(7, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(6, size0 = nodesetGroup.getSize());

	EXPECT_EQ(OK, result = facesMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(0, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(11, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(20, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(12, size0 = nodesetGroup.getSize());

	EXPECT_EQ(OK, result = facesMeshGroup.removeElementsConditional(!isOnFaceField));
	EXPECT_EQ(0, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(2, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(7, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(6, size0 = nodesetGroup.getSize());

	EXPECT_EQ(OK, result = facesMeshGroup.removeElementsConditional(facesGroup));
	EXPECT_EQ(0, result = elementsMeshGroup.getSize());
	EXPECT_EQ(0, result = facesMeshGroup.getSize());
	EXPECT_EQ(0, result = linesMeshGroup.getSize());
	EXPECT_EQ(0, result = nodesetGroup.getSize());

	// test removal of lines appropriately leaves nodes behind
	Element line1 = mesh1d.findElementByIdentifier(1);
	EXPECT_TRUE(line1.isValid());
	Element line3 = mesh1d.findElementByIdentifier(3);
	EXPECT_TRUE(line3.isValid());
	Element line13 = mesh1d.findElementByIdentifier(13);
	EXPECT_TRUE(line13.isValid());
	Element line14 = mesh1d.findElementByIdentifier(14);
	EXPECT_TRUE(line14.isValid());

	EXPECT_EQ(OK, result = linesMeshGroup.addElement(line1));
	EXPECT_EQ(OK, result = linesMeshGroup.addElement(line3));
	EXPECT_EQ(OK, result = linesMeshGroup.addElement(line13));
	EXPECT_EQ(OK, result = linesMeshGroup.addElement(line14));
	EXPECT_EQ(0, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(0, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(4, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(4, size0 = nodesetGroup.getSize());
	EXPECT_EQ(OK, result = linesMeshGroup.removeElement(line1));
	EXPECT_EQ(3, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(4, size0 = nodesetGroup.getSize());

	// fill group, ready to test clear()
	EXPECT_EQ(OK, result = elementsMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(2, size3 = elementsMeshGroup.getSize());
	EXPECT_EQ(11, size2 = facesMeshGroup.getSize());
	EXPECT_EQ(20, size1 = linesMeshGroup.getSize());
	EXPECT_EQ(12, size0 = nodesetGroup.getSize());

	// remove handles to subobject group fields/domains so they can be cleaned up
	char *name;
	name = elementsMeshGroup.getName();
	EXPECT_STREQ("group.mesh3d", name);
	cmzn_deallocate(name);
	name = nodesetGroup.getName();
	EXPECT_STREQ("group.nodes", name);
	cmzn_deallocate(name);
	elementsGroup = FieldElementGroup();
	elementsMeshGroup = MeshGroup();
	facesGroup = FieldElementGroup();
	facesMeshGroup = MeshGroup();
	linesGroup = FieldElementGroup();
	linesMeshGroup = MeshGroup();
	nodesGroup = FieldNodeGroup();
	nodesetGroup = NodesetGroup();

	// check clear empties and orphans subobject groups
	Mesh tempMesh;
	Nodeset tempNodeset;
	EXPECT_EQ(OK, result = group.clear());

	elementsGroup = group.getFieldElementGroup(mesh3d);
	EXPECT_FALSE(elementsGroup.isValid());
	tempMesh = zinc.fm.findMeshByName("group.mesh3d");
	EXPECT_FALSE(tempMesh.isValid());

	facesGroup = group.getFieldElementGroup(mesh2d);
	EXPECT_FALSE(facesGroup.isValid());
	tempMesh = zinc.fm.findMeshByName("group.mesh2d");
	EXPECT_FALSE(tempMesh.isValid());

	linesGroup = group.getFieldElementGroup(mesh1d);
	EXPECT_FALSE(linesGroup.isValid());
	tempMesh = zinc.fm.findMeshByName("group.mesh1d");
	EXPECT_FALSE(tempMesh.isValid());

	nodesGroup = group.getFieldNodeGroup(nodeset);
	EXPECT_FALSE(nodesGroup.isValid());
	tempNodeset = zinc.fm.findNodesetByName("group.nodes");
	EXPECT_FALSE(tempNodeset.isValid());
}

TEST(ZincMesh, castGroup)
{
	ZincTestSetupCpp zinc;
	int result;

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(CMZN_OK, result = group.setName("group"));

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	MeshGroup noMeshGroup = mesh3d.castGroup();
	EXPECT_FALSE(noMeshGroup.isValid());

	FieldElementGroup elementGroup = group.createFieldElementGroup(mesh3d);
	EXPECT_TRUE(elementGroup.isValid());

	MeshGroup expectedMeshGroup = elementGroup.getMeshGroup();
	EXPECT_TRUE(expectedMeshGroup.isValid());

	Mesh findMesh = zinc.fm.findMeshByName("group.mesh3d");
	EXPECT_TRUE(findMesh.isValid());

	MeshGroup findMeshGroup = findMesh.castGroup();
	EXPECT_TRUE(findMesh.isValid());

	EXPECT_EQ(expectedMeshGroup, findMeshGroup);
}

TEST(ZincNodeset, castGroup)
{
	ZincTestSetupCpp zinc;
	int result;

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(CMZN_OK, result = group.setName("group"));

	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	NodesetGroup noNodesetGroup = nodeset.castGroup();
	EXPECT_FALSE(noNodesetGroup.isValid());

	FieldNodeGroup nodeGroup = group.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(nodeGroup.isValid());

	NodesetGroup expectedNodesetGroup = nodeGroup.getNodesetGroup();
	EXPECT_TRUE(expectedNodesetGroup.isValid());

	Nodeset findNodeset = zinc.fm.findNodesetByName("group.nodes");
	EXPECT_TRUE(findNodeset.isValid());

	NodesetGroup findNodesetGroup = findNodeset.castGroup();
	EXPECT_TRUE(findNodeset.isValid());

	EXPECT_EQ(expectedNodesetGroup, findNodesetGroup);
}

namespace {

class SelectioncallbackRecordChange : public Selectioncallback
{
	int changeFlags;

	virtual void operator()(const Selectionevent &selectionevent)
	{
		this->changeFlags = selectionevent.getChangeFlags();
	}

public:
	SelectioncallbackRecordChange() :
			Selectioncallback(),
			changeFlags(Selectionevent::CHANGE_FLAG_NONE)
	{ }

	void clear()
	{
		changeFlags = Selectionevent::CHANGE_FLAG_NONE;
	}

	int getChangeSummary() const
	{
		return this->changeFlags;
	}

};

} // anonymous namespace

// Test that emptying subobject or subregion groups then removing empty
// subgroups - while fieldmodule changes are stopped - correctly propagates
// a change. There was a defect where this was reported as no change.
TEST(ZincFieldGroup, removeEmptySubgroupsPropagatesChanges)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/two_cubes.exformat").c_str()));

	int size0, size3;
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(2, size3 = mesh3d.getSize());
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(12, size0 = nodeset.getSize());

	Element element1 = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	Node node1 = nodeset.findNodeByIdentifier(1);
	EXPECT_TRUE(node1.isValid());

	// set up notification of scene selection group changes
	
	Selectionnotifier selectionnotifier = zinc.scene.createSelectionnotifier();
	EXPECT_TRUE(selectionnotifier.isValid());

	SelectioncallbackRecordChange callback;
	EXPECT_EQ(OK, result = selectionnotifier.setCallback(callback));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(OK, result = group.setName("group"));

	FieldElementGroup elementGroup = group.createFieldElementGroup(mesh3d);
	EXPECT_TRUE(elementGroup.isValid());
	MeshGroup meshGroup = elementGroup.getMeshGroup();
	EXPECT_TRUE(meshGroup.isValid());
	EXPECT_EQ(0, meshGroup.getSize());
	EXPECT_EQ(OK, meshGroup.addElement(element1));

	// test that setting a non-empty selection group notifies
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_NONE, result = callback.getChangeSummary());
	EXPECT_EQ(OK, zinc.scene.setSelectionField(group));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_ADD, result = callback.getChangeSummary());

	// test that removing an element correctly propagates to the parent group
	callback.clear();
	EXPECT_EQ(OK, meshGroup.removeElement(element1));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());

	// test that re-adding an element correctly propagates to the parent group
	callback.clear();
	EXPECT_EQ(OK, meshGroup.addElement(element1));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_ADD, result = callback.getChangeSummary());

	// test that removing an element and flushing empty subgroups while
	// caching changes correctly propagates to the parent group
	callback.clear();
	zinc.fm.beginChange();
	EXPECT_EQ(OK, meshGroup.removeElement(element1));
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_NONE, result = callback.getChangeSummary());
	zinc.fm.endChange();
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());
	// rediscover orphaned element/mesh group
	EXPECT_TRUE((elementGroup = group.getFieldElementGroup(mesh3d)).isValid());
	EXPECT_TRUE((meshGroup = elementGroup.getMeshGroup()).isValid());

	FieldNodeGroup nodeGroup = group.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(nodeGroup.isValid());
	NodesetGroup nodesetGroup = nodeGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());

	// test that adding a node correctly propagates to the parent group
	callback.clear();
	EXPECT_EQ(OK, nodesetGroup.addNode(node1));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_ADD, result = callback.getChangeSummary());

	// test that removing node correctly propagates to the parent group
	callback.clear();
	EXPECT_EQ(OK, nodesetGroup.removeNode(node1));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());
	EXPECT_EQ(OK, nodesetGroup.addNode(node1)); // re-add node

	// test that removing a node and flushing empty subgroups while
	// caching changes correctly propagates to the parent group
	callback.clear();
	zinc.fm.beginChange();
	EXPECT_EQ(OK, nodesetGroup.removeNode(node1));
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_NONE, result = callback.getChangeSummary());
	zinc.fm.endChange();
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());
	// rediscover orphaned node/nodest group
	EXPECT_TRUE((nodeGroup = group.getFieldNodeGroup(nodeset)).isValid());
	EXPECT_TRUE((nodesetGroup = nodeGroup.getNodesetGroup()).isValid());

	// test propagation of change from adding a child region to group
	Region childRegion = zinc.root_region.createChild("child");
	EXPECT_TRUE(childRegion.isValid());
	FieldGroup childGroup = group.getSubregionFieldGroup(childRegion);
	EXPECT_FALSE(childGroup.isValid());
	callback.clear();
	EXPECT_EQ(OK, result = group.addRegion(childRegion));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_ADD, result = callback.getChangeSummary());
	childGroup = group.getSubregionFieldGroup(childRegion);
	EXPECT_TRUE(childGroup.isValid());

	// test propagation of change from removing child region
	callback.clear();
	EXPECT_EQ(OK, result = group.removeRegion(childRegion));
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());
	EXPECT_EQ(OK, result = group.addRegion(childRegion));

	// test that removing child region and flushing empty subgroups while
	// caching changes correctly propagates to the parent group
	callback.clear();
	zinc.fm.beginChange();
	childGroup = FieldGroup();  // so not rediscovered by name
	EXPECT_EQ(OK, result = group.removeRegion(childRegion));
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_NONE, result = callback.getChangeSummary());
	zinc.fm.endChange();
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());

	childGroup = group.getSubregionFieldGroup(childRegion);
	EXPECT_FALSE(childGroup.isValid());

	EXPECT_EQ(OK, result = group.addRegion(childRegion));
	childGroup = group.getSubregionFieldGroup(childRegion);
	EXPECT_TRUE(childGroup.isValid());

	// test that removing child region local group and flushing empty subgroups while
	// caching hierarchical changes correctly propagates to the parent group
	callback.clear();
	zinc.root_region.beginHierarchicalChange();
	EXPECT_EQ(OK, result = childGroup.clearLocal());
	EXPECT_EQ(OK, group.removeEmptySubgroups());
	childGroup = FieldGroup();
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_NONE, result = callback.getChangeSummary());
	zinc.root_region.endHierarchicalChange();
	EXPECT_EQ(Selectionevent::CHANGE_FLAG_REMOVE, result = callback.getChangeSummary());
}

// Issue 3852: Check changes to group propagate to fields depending on mesh/nodeset
TEST(ZincFieldGroup, issue_3852_dependency_on_mesh_or_nodeset_group)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/cube.exformat").c_str()));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());

	Mesh mesh = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh.isValid());
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(OK, result = group.setName("group"));
	EXPECT_EQ(OK, result = group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));

	FieldElementGroup elementGroup = group.createFieldElementGroup(mesh);
	EXPECT_TRUE(elementGroup.isValid());
	MeshGroup meshGroup = elementGroup.getMeshGroup();
	EXPECT_TRUE(meshGroup.isValid());

	FieldNodeGroup nodeGroup = group.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(nodeGroup.isValid());
	NodesetGroup nodesetGroup = nodeGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());

	Element element1 = mesh.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	EXPECT_EQ(OK, result = meshGroup.addElement(element1));

	const double valueOne = 1.0;
	Field one = zinc.fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral groupArea = zinc.fm.createFieldMeshIntegral(one, coordinates, meshGroup);
	EXPECT_TRUE(groupArea.isValid());
	FieldNodesetSum groupNodes = zinc.fm.createFieldNodesetSum(one, nodesetGroup);
	EXPECT_TRUE(groupNodes.isValid());
	FieldFindMeshLocation findMeshLocation = zinc.fm.createFieldFindMeshLocation(coordinates, coordinates, meshGroup);
	EXPECT_TRUE(findMeshLocation.isValid());
	EXPECT_EQ(OK, result = findMeshLocation.setSearchMode(FieldFindMeshLocation::SEARCH_MODE_NEAREST));

	Fieldcache cache = zinc.fm.createFieldcache();
	Node node6 = nodeset.findNodeByIdentifier(6);
	EXPECT_TRUE(node6.isValid());
	EXPECT_EQ(OK, result = cache.setNode(node6));
	double area, count, findXi[2];
	Element findElement;

	EXPECT_EQ(OK, result = groupArea.evaluateReal(cache, 1, &area));
	EXPECT_DOUBLE_EQ(1.0, area);
	EXPECT_EQ(OK, result = groupNodes.evaluateReal(cache, 1, &count));
	EXPECT_DOUBLE_EQ(4.0, count);
	findElement = findMeshLocation.evaluateMeshLocation(cache, 2, findXi);
	EXPECT_EQ(element1, findElement);
	EXPECT_DOUBLE_EQ(0.0, findXi[0]);
	EXPECT_DOUBLE_EQ(1.0, findXi[1]);

	Element element3 = mesh.findElementByIdentifier(3);
	EXPECT_TRUE(element3.isValid());
	EXPECT_EQ(OK, result = meshGroup.addElement(element3));

	EXPECT_EQ(OK, result = groupArea.evaluateReal(cache, 1, &area));
	EXPECT_DOUBLE_EQ(2.0, area);
	EXPECT_EQ(OK, result = groupNodes.evaluateReal(cache, 1, &count));
	EXPECT_DOUBLE_EQ(6.0, count);
	findElement = findMeshLocation.evaluateMeshLocation(cache, 2, findXi);
	EXPECT_EQ(element3, findElement);
	EXPECT_DOUBLE_EQ(1.0, findXi[0]);
	EXPECT_DOUBLE_EQ(1.0, findXi[1]);
};

TEST(ZincFieldElementGroup, ElementiteratorInvalidation)
{
	ZincTestSetupCpp zinc;

	Region childRegion = zinc.root_region.createChild("temp");
	Fieldmodule fm = childRegion.getFieldmodule();

	Mesh mesh = fm.findMeshByDimension(3);
	Elementtemplate elementTemplate = mesh.createElementtemplate();
	EXPECT_TRUE(elementTemplate.isValid());
	EXPECT_EQ(OK, elementTemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));

	FieldElementGroup elementGroupField = fm.createFieldElementGroup(mesh);
	EXPECT_TRUE(elementGroupField.isValid());
	MeshGroup meshGroup = elementGroupField.getMeshGroup();
	EXPECT_TRUE(meshGroup.isValid());

	Element element[32];
	for (int e = 0; e < 31; ++e)
	{
		element[e] = meshGroup.createElement(e + 1, elementTemplate);
		EXPECT_TRUE(element[e].isValid());
	}
	EXPECT_EQ(31, meshGroup.getSize());

	Elementiterator iter;
	Element el;

	// test that creating a new element safely invalidates iterator
	iter = meshGroup.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	el = iter.next();
	EXPECT_EQ(element[0], el);
	el = iter.next();
	EXPECT_EQ(element[1], el);
	element[31] = meshGroup.createElement(32, elementTemplate);
	EXPECT_TRUE(element[31].isValid());
	EXPECT_EQ(32, meshGroup.getSize());
	el = iter.next();
	EXPECT_FALSE(el.isValid());

	// test that removing an element safely invalidates iterator
	elementTemplate = Elementtemplate();
	fm = Fieldmodule();
	iter = meshGroup.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	el = iter.next();
	EXPECT_EQ(element[0], el);
	el = iter.next();
	EXPECT_EQ(element[1], el);
	EXPECT_EQ(OK, meshGroup.removeElement(element[3]));
	EXPECT_EQ(31, meshGroup.getSize());
	el = iter.next();
	EXPECT_FALSE(el.isValid());

	// test that destroying an element (even if not in group) safely invalidates iterator
	elementTemplate = Elementtemplate();
	fm = Fieldmodule();
	iter = meshGroup.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	el = iter.next();
	EXPECT_EQ(element[0], el);
	el = iter.next();
	EXPECT_EQ(element[1], el);
	EXPECT_EQ(OK, mesh.destroyElement(element[5]));
	EXPECT_EQ(31, mesh.getSize());
	EXPECT_EQ(30, meshGroup.getSize());
	Mesh tmpMesh = element[5].getMesh();
	EXPECT_FALSE(tmpMesh.isValid());
	el = iter.next();
	EXPECT_FALSE(el.isValid());

	// test that renumbering an element safely invalidates iterator
	iter = meshGroup.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	el = iter.next();
	EXPECT_EQ(element[0], el);
	el = iter.next();
	EXPECT_EQ(element[1], el);
	EXPECT_EQ(OK, element[31].setIdentifier(55));
	el = iter.next();
	EXPECT_FALSE(el.isValid());

	// test that destroying child region safely invalidates iterator
	iter = meshGroup.createElementiterator();
	EXPECT_TRUE(iter.isValid());
	el = iter.next();
	EXPECT_EQ(element[0], el);
	el = iter.next();
	EXPECT_EQ(element[1], el);
	EXPECT_EQ(OK, zinc.root_region.removeChild(childRegion));
	childRegion = Region(); // clear handle so it can be destroyed
	meshGroup = MeshGroup(); // clear handle
	elementGroupField = FieldElementGroup(); // clear handle
	mesh = Mesh(); // clear handle
	el = iter.next();
	EXPECT_FALSE(el.isValid());
	tmpMesh = element[15].getMesh();
	EXPECT_FALSE(tmpMesh.isValid());
}

TEST(ZincFieldNodeGroup, NodeiteratorInvalidation)
{
	ZincTestSetupCpp zinc;

	Region childRegion = zinc.root_region.createChild("temp");
	Fieldmodule fm = childRegion.getFieldmodule();

	Nodeset nodeset = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	Nodetemplate nodeTemplate = nodeset.createNodetemplate();
	EXPECT_TRUE(nodeTemplate.isValid());

	FieldNodeGroup nodeGroupField = fm.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(nodeGroupField.isValid());
	NodesetGroup nodesetGroup = nodeGroupField.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());

	Node node[32];
	for (int n = 0; n < 31; ++n)
	{
		node[n] = nodesetGroup.createNode(n + 1, nodeTemplate);
		EXPECT_TRUE(node[n].isValid());
	}
	EXPECT_EQ(31, nodesetGroup.getSize());

	Nodeiterator iter;
	Node nd;

	// test that creating a new node safely invalidates iterator
	iter = nodesetGroup.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	nd = iter.next();
	EXPECT_EQ(node[0], nd);
	nd = iter.next();
	EXPECT_EQ(node[1], nd);
	node[31] = nodesetGroup.createNode(32, nodeTemplate);
	EXPECT_TRUE(node[31].isValid());
	EXPECT_EQ(32, nodesetGroup.getSize());
	nd = iter.next();
	EXPECT_FALSE(nd.isValid());

	// test that removing a node safely invalidates iterator
	nodeTemplate = Nodetemplate();
	fm = Fieldmodule();
	iter = nodesetGroup.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	nd = iter.next();
	EXPECT_EQ(node[0], nd);
	nd = iter.next();
	EXPECT_EQ(node[1], nd);
	EXPECT_EQ(OK, nodesetGroup.removeNode(node[3]));
	EXPECT_EQ(31, nodesetGroup.getSize());
	nd = iter.next();
	EXPECT_FALSE(nd.isValid());

	// test that destroying a node (even if not in group) safely invalidates iterator
	nodeTemplate = Nodetemplate();
	fm = Fieldmodule();
	iter = nodesetGroup.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	nd = iter.next();
	EXPECT_EQ(node[0], nd);
	nd = iter.next();
	EXPECT_EQ(node[1], nd);
	EXPECT_EQ(OK, nodeset.destroyNode(node[5]));
	EXPECT_EQ(31, nodeset.getSize());
	EXPECT_EQ(30, nodesetGroup.getSize());
	Nodeset tmpNodeset = node[5].getNodeset();
	EXPECT_FALSE(tmpNodeset.isValid());
	nd = iter.next();
	EXPECT_FALSE(nd.isValid());

	// test that renumbering a node safely invalidates iterator
	iter = nodesetGroup.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	nd = iter.next();
	EXPECT_EQ(node[0], nd);
	nd = iter.next();
	EXPECT_EQ(node[1], nd);
	EXPECT_EQ(OK, node[31].setIdentifier(55));
	nd = iter.next();
	EXPECT_FALSE(nd.isValid());

	// test that destroying child region safely invalidates iterator
	iter = nodesetGroup.createNodeiterator();
	EXPECT_TRUE(iter.isValid());
	nd = iter.next();
	EXPECT_EQ(node[0], nd);
	nd = iter.next();
	EXPECT_EQ(node[1], nd);
	EXPECT_EQ(OK, zinc.root_region.removeChild(childRegion));
	childRegion = Region(); // clear handle so it can be destroyed
	nodesetGroup = NodesetGroup(); // clear handle
	nodeGroupField = FieldNodeGroup(); // clear handle
	nodeset = Nodeset(); // clear handle
	nd = iter.next();
	EXPECT_FALSE(nd.isValid());
	tmpNodeset = node[15].getNodeset();
	EXPECT_FALSE(tmpNodeset.isValid());
}

// test bug in manager cleanup with active change cache which leads to
// subgroup being removed from changed object list while it is iterated over
TEST(ZincFieldGroup, fieldCleanupOrder)
{
	ZincTestSetupCpp zinc;

	zinc.fm.beginChange();

	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());
	EXPECT_EQ(OK, group.setName("group"));
	EXPECT_EQ(OK, group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	EXPECT_EQ(RESULT_OK, group.setManaged(true));

	FieldElementGroup elementGroup = group.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(elementGroup.isValid());
}

// Test bug where main group with subelement groups falls to zero access count
// while change cache is in effect. The subelement groups remain linked to their owner
// group and vice-versa. If another group is created and the subelement groups
// rediscovered, their owner pointers are cleared when the original owner group is
// later destroyed.
TEST(ZincFieldGroup, orphaned_subelement_groups)
{
	ZincTestSetupCpp zinc;
	char* name;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/cube.exformat").c_str()));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_EQ(RESULT_OK, group.setName("bob"));
	EXPECT_EQ(RESULT_OK, group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));

	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	FieldElementGroup elementGroup2d = group.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(elementGroup2d.isValid());
	name = elementGroup2d.getName();
	EXPECT_STREQ("bob.mesh2d", name);
	cmzn_deallocate(name);
	MeshGroup meshGroup2d = elementGroup2d.getMeshGroup();
	EXPECT_TRUE(meshGroup2d.isValid());
	for (int nid = 2; nid <= 4; ++nid)
	{
		EXPECT_EQ(RESULT_OK, meshGroup2d.addElement(mesh2d.findElementByIdentifier(nid)));
	}
	EXPECT_EQ(3, meshGroup2d.getSize());

	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	FieldElementGroup elementGroup1d = group.getFieldElementGroup(mesh1d);
	EXPECT_TRUE(elementGroup1d.isValid());
	name = elementGroup1d.getName();
	EXPECT_STREQ("bob.mesh1d", name);
	cmzn_deallocate(name);
	MeshGroup meshGroup1d = elementGroup1d.getMeshGroup();
	EXPECT_TRUE(meshGroup1d.isValid());
	EXPECT_EQ(10, meshGroup1d.getSize());

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	FieldNodeGroup nodeGroup = group.getFieldNodeGroup(nodes);
	EXPECT_TRUE(nodeGroup.isValid());
	name = nodeGroup.getName();
	EXPECT_STREQ("bob.nodes", name);
	cmzn_deallocate(name);
	NodesetGroup nodesetGroup = nodeGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());
	EXPECT_EQ(8, nodesetGroup.getSize());

	EXPECT_EQ(RESULT_OK, zinc.fm.beginChange());
	// clear group to orphan its subelement groups
	group = FieldGroup();
	Field tmpField = zinc.fm.findFieldByName("bob");
	EXPECT_FALSE(tmpField.isValid());
	tmpField = zinc.fm.findFieldByName("bob.mesh2d");
	EXPECT_EQ(elementGroup2d, tmpField);
	tmpField = zinc.fm.findFieldByName("bob.mesh1d");
	EXPECT_EQ(elementGroup1d, tmpField);
	tmpField = zinc.fm.findFieldByName("bob.nodes");
	EXPECT_EQ(nodeGroup, tmpField);
	// now create another group of the same name and find the subelement groups
	// which aren't cleared because this test still holds a handle to them
	group = zinc.fm.createFieldGroup();
	EXPECT_EQ(RESULT_OK, group.setName("bob"));
	EXPECT_EQ(RESULT_OK, group.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
	tmpField = group.getFieldElementGroup(mesh2d);
	EXPECT_EQ(elementGroup2d, tmpField);
	EXPECT_EQ(3, meshGroup2d.getSize());
	tmpField = group.getFieldElementGroup(mesh1d);
	EXPECT_EQ(elementGroup1d, tmpField);
	EXPECT_EQ(10, meshGroup1d.getSize());
	tmpField = group.getFieldNodeGroup(nodes);
	EXPECT_EQ(nodeGroup, tmpField);
	EXPECT_EQ(8, nodesetGroup.getSize());
	EXPECT_EQ(RESULT_OK, zinc.fm.endChange());

	// get meshGroup3d
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	FieldElementGroup elementGroup3d = group.createFieldElementGroup(mesh3d);
	EXPECT_TRUE(elementGroup3d.isValid());
	name = elementGroup3d.getName();
	EXPECT_STREQ("bob.mesh3d", name);
	cmzn_deallocate(name);
	MeshGroup meshGroup3d = elementGroup3d.getMeshGroup();
	EXPECT_TRUE(meshGroup3d.isValid());
	EXPECT_EQ(0, meshGroup3d.getSize());

	// following formerly crashed due to nullptr to owner group in subelement groups
	EXPECT_EQ(RESULT_OK, meshGroup3d.addElement(mesh3d.findElementByIdentifier(1)));
	EXPECT_EQ(1, meshGroup3d.getSize());
	EXPECT_EQ(6, meshGroup2d.getSize());
	EXPECT_EQ(12, meshGroup1d.getSize());
	EXPECT_EQ(8, nodesetGroup.getSize());

	// now test what happens if we don't hold handles to the subelement groups and mesh groups
	elementGroup3d = FieldElementGroup();
	meshGroup3d = MeshGroup();
	elementGroup2d = FieldElementGroup();
	meshGroup2d = MeshGroup();
	elementGroup1d = FieldElementGroup();
	meshGroup1d = MeshGroup();
	nodeGroup = FieldNodeGroup();
	nodesetGroup = NodesetGroup();
	tmpField = Field();
	EXPECT_EQ(RESULT_OK, zinc.fm.beginChange());
	// clear group to orphan its subelement groups
	group = FieldGroup();
	tmpField = zinc.fm.findFieldByName("bob");
	EXPECT_FALSE(tmpField.isValid());
	tmpField = zinc.fm.findFieldByName("bob.mesh3d");
	EXPECT_FALSE(tmpField.isValid());
	tmpField = zinc.fm.findFieldByName("bob.mesh2d");
	EXPECT_FALSE(tmpField.isValid());
	tmpField = zinc.fm.findFieldByName("bob.mesh1d");
	EXPECT_FALSE(tmpField.isValid());
	tmpField = zinc.fm.findFieldByName("bob.nodes");
	EXPECT_FALSE(tmpField.isValid());
	EXPECT_EQ(RESULT_OK, zinc.fm.endChange());
}

// Test get/create subobject groups in subregions.
TEST(ZincFieldGroup, getCreateSubregionSubobjectGroups)
{
	ZincTestSetupCpp zinc;
	char* name;
	Field tmpField;

	Region child = zinc.root_region.createChild("child");
	EXPECT_TRUE(child.isValid());

	FieldGroup bob = zinc.fm.createFieldGroup();
	EXPECT_TRUE(bob.isValid());
	EXPECT_EQ(RESULT_OK, bob.setName("bob"));
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	FieldElementGroup bobMesh2d = bob.getFieldElementGroup(mesh2d);
	EXPECT_FALSE(bobMesh2d.isValid());
	bobMesh2d = bob.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(bobMesh2d.isValid());
	name = bobMesh2d.getName();
	EXPECT_STREQ("bob.mesh2d", name);
	cmzn_deallocate(name);
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	FieldNodeGroup bobNodes = bob.getFieldNodeGroup(nodes);
	EXPECT_FALSE(bobNodes.isValid());
	bobNodes = bob.createFieldNodeGroup(nodes);
	EXPECT_TRUE(bobNodes.isValid());
	name = bobNodes.getName();
	EXPECT_STREQ("bob.nodes", name);
	cmzn_deallocate(name);

	FieldGroup childBob = bob.getSubregionFieldGroup(child);
	EXPECT_FALSE(childBob.isValid());
	childBob = bob.createSubregionFieldGroup(child);
	EXPECT_TRUE(childBob.isValid());
	name = childBob.getName();
	EXPECT_STREQ("bob", name);
	cmzn_deallocate(name);
	Fieldmodule childFm = child.getFieldmodule();
	Mesh childMesh2d = childFm.findMeshByDimension(2);
	FieldElementGroup childBobMesh2d = bob.getFieldElementGroup(childMesh2d);
	EXPECT_FALSE(childBobMesh2d.isValid());
	childBobMesh2d = bob.createFieldElementGroup(childMesh2d);
	EXPECT_TRUE(childBobMesh2d.isValid());
	name = childBobMesh2d.getName();
	EXPECT_STREQ("bob.mesh2d", name);
	cmzn_deallocate(name);
	Nodeset childNodes = childFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	FieldNodeGroup childBobNodes = bob.getFieldNodeGroup(childNodes);
	EXPECT_FALSE(childBobNodes.isValid());
	childBobNodes = bob.createFieldNodeGroup(childNodes);
	EXPECT_TRUE(childBobNodes.isValid());
	name = childBobNodes.getName();
	EXPECT_STREQ("bob.nodes", name);
	cmzn_deallocate(name);

	// orphan subobject groups in child region and try to get them back by name
	childBob = FieldGroup();
	bob.clear();
	bob.removeEmptySubgroups();
	tmpField = childFm.findFieldByName("bob");
	EXPECT_FALSE(tmpField.isValid());
	tmpField = childFm.findFieldByName("bob.mesh2d");
	EXPECT_TRUE(tmpField.isValid());
	tmpField = childFm.findFieldByName("bob.nodes");
	EXPECT_TRUE(tmpField.isValid());
	tmpField = Field();
	FieldElementGroup childBobMesh2dGet = bob.getFieldElementGroup(childMesh2d);
	EXPECT_TRUE(childBobMesh2dGet.isValid());
	EXPECT_EQ(childBobMesh2d, childBobMesh2dGet);
	name = childBobMesh2dGet.getName();
	EXPECT_STREQ("bob.mesh2d", name);
	cmzn_deallocate(name);
	FieldNodeGroup childBobNodesTmp = bob.getFieldNodeGroup(childNodes);
	EXPECT_TRUE(childBobNodesTmp.isValid());
	EXPECT_EQ(childBobNodes, childBobNodesTmp);
	name = childBobNodesTmp.getName();
	EXPECT_STREQ("bob.nodes", name);
	cmzn_deallocate(name);
	tmpField = childFm.findFieldByName("bob");
	EXPECT_TRUE(tmpField.isValid());
	tmpField = Field();

	// orphan subobject groups in child region and test can't create them
	childBob = FieldGroup();
	bob.clear();
	bob.removeEmptySubgroups();
	tmpField = childFm.findFieldByName("bob");
	EXPECT_FALSE(tmpField.isValid());
	tmpField = childFm.findFieldByName("bob.mesh2d");
	EXPECT_TRUE(tmpField.isValid());
	tmpField = childFm.findFieldByName("bob.nodes");
	EXPECT_TRUE(tmpField.isValid());
	tmpField = Field();
	FieldElementGroup childBobMesh2dCreate = bob.createFieldElementGroup(childMesh2d);
	EXPECT_FALSE(childBobMesh2dCreate.isValid());
	FieldNodeGroup childBobNodesCreate = bob.createFieldNodeGroup(childNodes);
	EXPECT_FALSE(childBobNodesCreate.isValid());
	// as a side effect, the subregion group is created:
	tmpField = childFm.findFieldByName("bob");
	EXPECT_TRUE(tmpField.isValid());
	tmpField = Field();

	// test can't get a subregion subobject group if existing field of wrong type is using expected name
	FieldGroup fred = zinc.fm.createFieldGroup();
	EXPECT_TRUE(fred.isValid());
	EXPECT_EQ(RESULT_OK, fred.setName("fred"));
	const double one = 1.0;
	FieldConstant childFredMesh2dConstant = childFm.createFieldConstant(1, &one);
	EXPECT_TRUE(childFredMesh2dConstant.isValid());
	EXPECT_EQ(RESULT_OK, childFredMesh2dConstant.setName("fred.mesh2d"));
	FieldElementGroup childFredMesh2dErr = fred.getFieldElementGroup(childMesh2d);
	EXPECT_FALSE(childFredMesh2dErr.isValid());
	childFredMesh2dErr = fred.createFieldElementGroup(childMesh2d);
	EXPECT_FALSE(childFredMesh2dErr.isValid());
	FieldConstant childFredNodesConstant = childFm.createFieldConstant(1, &one);
	EXPECT_TRUE(childFredNodesConstant.isValid());
	EXPECT_EQ(RESULT_OK, childFredNodesConstant.setName("fred.nodes"));
	FieldNodeGroup childFredNodesErr = fred.getFieldNodeGroup(childNodes);
	EXPECT_FALSE(childFredNodesErr.isValid());
	childFredNodesErr = fred.createFieldNodeGroup(childNodes);
	EXPECT_FALSE(childFredNodesErr.isValid());
	// as a side effect, the subregion group is created:
	tmpField = childFm.findFieldByName("fred");
	EXPECT_TRUE(tmpField.isValid());
	tmpField = Field();
}
