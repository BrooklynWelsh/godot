/*************************************************************************/
/*  test_node.h                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef TEST_NODE_H
#define TEST_NODE_H

#include "scene/main/node.h"
#include "tests/test_macros.h"
#include "core/core_bind.h"
#include "scene/resources/packed_scene.h"

#include "thirdparty/doctest/doctest.h"

namespace TestNode {
TEST_SUITE("[Node]") {
	TEST_CASE("[Node] Core getters") {
		Node node;

		CHECK(node.is_class("Node"));
		CHECK(node.get_class() == "Node");
		CHECK(node.get_class_name() == "Node");
		CHECK(node.get_class_static() == "Node");
		CHECK(node.get_save_class() == "Node");
	}

	TEST_CASE("[Node] Test parent/children logic") {
		Node grandparent;
		Node parent;
		Node firstChild;
		Node secondChild;

		// Initial child count
		CHECK(grandparent.get_child_count() == 0);
		CHECK(parent.get_child_count() == 0);
		CHECK(firstChild.get_child_count() == 0);
		CHECK(secondChild.get_child_count() == 0);

		// get_child_count() after adding children
		grandparent.add_child(&parent);
		parent.add_child(&firstChild);
		parent.add_child(&secondChild);
		CHECK(grandparent.get_child_count() == 1);
		CHECK(parent.get_child_count() == 2);

		// get_index()
		CHECK(grandparent.get_index() == -1);
		CHECK(firstChild.get_index() == 0);
		CHECK(secondChild.get_index() == 1);

		// move_child() && get_index()
		parent.move_child(&secondChild, 0);
		CHECK(firstChild.get_index() == 1);
		CHECK(secondChild.get_index() == 0);

		// add_sibling , get_child(), get_child_count()
		parent.remove_child(&secondChild);
		firstChild.add_sibling(&secondChild);
		CHECK(parent.get_child(0) == &firstChild);
		CHECK(parent.get_child(1) == &secondChild);
		CHECK(firstChild.get_child_count() == 0);
		CHECK(secondChild.get_child_count() == 0);
		CHECK(grandparent.get_child(0) == &parent);

		// is_a_parent_of()
		CHECK(grandparent.is_a_parent_of(&parent));
		CHECK(!parent.is_a_parent_of(&grandparent));

		// find_common_parent_with()
		CHECK(grandparent.find_common_parent_with(&parent) == &grandparent);
		CHECK(firstChild.find_common_parent_with(&secondChild) == &parent);

		// raise()
		secondChild.raise();
		CHECK(firstChild.get_index() == 0);
		CHECK(secondChild.get_index() == 1);

		// owner functions
		secondChild.set_owner(&grandparent);
		CHECK(secondChild.get_owner() == &grandparent);
		firstChild.set_owner(&grandparent);
		List<Node *> ownedNodes;
		grandparent.get_owned_by(&grandparent, &ownedNodes);
		CHECK(ownedNodes.size() == 2);

		// remove_and_skip()

		// remove_child()
		parent.remove_child(&secondChild);
		CHECK(parent.get_child_count() == 1);
	}

	TEST_CASE("[Node] Group logic") {
		Node node1;
		Node node2;
		List<Node::GroupInfo> groupList;

		// get_groups() null case
		node1.get_groups(&groupList);
		CHECK(!groupList.front());

		// add_to_group() & get_groups()
		node1.add_to_group("Test Group", false);
		node1.add_to_group("Another Group", true);
		node1.get_groups(&groupList);
		CHECK(groupList.size() == 2);

		// is_in_group()
		CHECK(node1.is_in_group("Test Group"));

		// get_persistent_group_count()
		CHECK(node1.get_persistent_group_count() == 1);

		// remove_from_group()
		node1.remove_from_group("Test Group");
		node1.remove_from_group("Another Group");
		groupList.clear();
		node1.get_groups(&groupList);
		CHECK(groupList.size() == 0);
	}

	TEST_CASE("[Node] Property getters and setters") {
		Node node;

		CHECK(node.get_name() == StringName(""));
		node.set_name("Test Name");
		CHECK(node.get_name() == StringName("Test Name"));

		// filename functions
		node.set_filename("Test Scene.tscn");
		CHECK(node.get_filename() == "Test Scene.tscn");

		// set_editor_description
		node.set_editor_description("This is an editor description");
		CHECK(node.get_editor_description() == "This is an editor description");

		// set_editable_instance, TODO needs to check on a child node
		Node child;
		node.add_child(&child);
		node.set_editable_instance(&child, true);
		CHECK(node.is_editable_instance(&child));

		// set_pause_mode
		CHECK(node.get_pause_mode() == Node::PAUSE_MODE_INHERIT);
		node.set_pause_mode(Node::PAUSE_MODE_STOP);
		CHECK(node.get_pause_mode() == Node::PAUSE_MODE_STOP);

		// set_import_path
		node.set_import_path(NodePath("Some/Path"));
		CHECK(node.get_import_path() == NodePath("Some/Path"));

		// set_display_folded
		CHECK(!node.is_displayed_folded());
		node.set_display_folded(true);
		CHECK(node.is_displayed_folded());

		// set_network_master, is_network_master
		int id = (uint64_t)node.get_instance_id();
		node.set_network_master(id);
		CHECK(node.get_network_master() == id);

		// get_custom_multiplayer, set_custom_multiplayer
		Ref<MultiplayerAPI> api = memnew(MultiplayerAPI);
		node.set_custom_multiplayer(api);
		api->set_root_node(&node);
		CHECK(node.get_custom_multiplayer()->get_root_node() == &node);
		memdelete(&api);
	}

	TEST_CASE("[Node] Processing functions") {
		// Can't really check much without SceneTree, just checking if it initialized correctly
		Node node;

		CHECK(!node.is_physics_processing());
		CHECK(!node.is_processing());
		CHECK(!node.is_physics_processing_internal());
		CHECK(!node.is_processing_internal());
		CHECK(!node.is_processing_input());
		CHECK(!node.is_processing_unhandled_input());
		CHECK(!node.is_processing_unhandled_key_input());
	}

	TEST_CASE("[Node] Duplicate") {
		Node node;
		node.set_name("Node");
		SUBCASE("Duplicate with no flags") {
			Node newNode = *node.duplicate();
			CHECK(newNode.get_name() == "Node");
		}

		SUBCASE("Duplicate groups") {
			node.add_to_group("New Group", true);
			Node newNode = *node.duplicate();
			List<Node::GroupInfo> groupList;
			newNode.get_groups(&groupList);
			CHECK(groupList.front()->get().name == "New Group");
			CHECK(newNode.get_persistent_group_count() == 1);
		}
	}

	TEST_CASE("[Node] Scene instance operations") {
		Node node;
		Ref<SceneState> state = memnew(SceneState);

		node.set_scene_instance_state(state);
		CHECK(node.get_scene_instance_state() == state);

		node.set_scene_inherited_state(state);
		CHECK(node.get_scene_inherited_state() == state);

		node.set_scene_instance_load_placeholder(true);
		CHECK(node.get_scene_instance_load_placeholder());
		memdelete(&state);
	}

	TEST_CASE("[Node] replace_by") {
		SUBCASE("Discard replaced node's groups") {
			Node parent;
			Node originalNode;
			Node originalChild;
			Node replacementNode;
			Node replacementChild;
			List<Node::GroupInfo> groups;

			parent.add_child(&originalNode);
			originalNode.add_child(&originalChild, false);
			originalNode.add_to_group("group", true);

			replacementNode.add_child(&replacementChild, false);
			originalNode.replace_by(&replacementNode, false);
			CHECK(parent.get_child(0) == &replacementNode);

			replacementNode.get_groups(&groups);
			CHECK(!groups.front());
		}

		SUBCASE("Keep replaced node's groups") {
			Node parent;
			Node originalNode;
			Node originalChild;
			Node replacementNode;
			Node replacementChild;
			List<Node::GroupInfo> groups;

			parent.add_child(&originalNode);
			originalNode.add_child(&originalChild, false);
			originalNode.add_to_group("group", true);
			replacementNode.add_child(&replacementChild, false);
			
			originalNode.replace_by(&replacementNode, true);
			CHECK(parent.get_child(0) == &replacementNode);
			CHECK(replacementNode.get_child(0) == &replacementChild);

			replacementNode.get_groups(&groups);
			CHECK(groups.front()->get().name == "group");
			CHECK(groups.front()->get().persistent);
		}
	}

	TEST_CASE("[Node] Print functions") {
		// Can't really test at the moment, just check for crashes.
		Node node;
		Node::print_stray_nodes();
		node.print_tree();
		node.print_tree_pretty();
		
	}

	TEST_CASE("[Node] force_parent_owned") {
		Node node;

		CHECK(!node.is_owned_by_parent());
		node.force_parent_owned();
		CHECK(node.is_owned_by_parent());
	}

	TEST_CASE("[Node] make_binds") {
		Node node;
		Variant var = Variant(5);

		Vector<Variant> vector = node.make_binds(var);
		CHECK(vector.has(&var));
	}

	TEST_CASE("[Node] validate_child_name") {
		Node node;
		Node child;
		Node anotherChild;
		Node yetAnotherChild;

		node.add_child(&child);
		node.add_child(&anotherChild);
		child.set_name("AName");
		anotherChild.set_name("AName");
		yetAnotherChild.set_name("DifferentName");

		CHECK_MESSAGE(node.validate_child_name(&child) != node.validate_child_name(&anotherChild),
						"validate_child_name should generate a unique name for nodes with duplicate names");
		CHECK_MESSAGE(node.validate_child_name(&anotherChild) != anotherChild.get_name(),
						"validate_child_name should generate a unique name for nodes with duplicate names");
		CHECK(node.validate_child_name(&yetAnotherChild) == yetAnotherChild.get_name());
	}
}
} // namespace TestNode
#endif // TEST_NODE_H
