# BT Demo Skills

Author: [Michael Terzer](mailto:michael.terzer@fraunhofer.it) - Fraunhofer Italia (2023)

This package provides a template for robotic skill implementations in form of behavior tree nodes to be used in combination with the [behavior_tree_stack](https://github.com/FraunhoferItalia/behavior_tree_stack).

The nodes implemented in this package, get registered and loaded by the plugin `bt_demo_skills::SkillLibrary`, which inherits from the [behavior_tree_stack](https://github.com/FraunhoferItalia/behavior_tree_stack) `bt_skill::SkillLibraryBase` class.

## How to create a new skill

1. Copy this package and rename it to `awesome_robotic_skill`.

2. Search `bt_demo_skills` and replace with `awesome_robotic_skill` in the package.

3. Include the package of the message, service or action type definition in the `CMakeLists.txt` and `package.xml`.

4. Substitute the file `publish_string.h` with `awesome_robotic_skill.h` as well as `publish_string.cpp` with `awesome_robotic_skill.cpp`.

5. Watch out to substitute the header guards, substitute 

```cpp
#ifndef PUBLISH_STRING_SKILL_LIBRARY
```

with:

```cpp
#ifndef AWESOME_ROBOTIC_SKILL_LIBRARY
```

6. Register the new node through the skill library load() function. It should now be possible for the **bt_skill_manager** in the **bt_executor** package to find the plugin and register the nodes in a behavior tree.

```cpp
void SkillLibrary::load(BT::BehaviorTreeFactory & factory, BT::RosNodeParams & params) const
{
  bt_skill::SkillLibraryBase::registerNodeType<AwesomeRoboticSkill>(factory, params);
  return;
}
```

7. Don't forget to compile the  `plugins.xml` with the dedicated skill library.
