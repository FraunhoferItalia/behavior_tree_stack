# https://github.com/ros2/design/pull/183#issuecomment-410094020
# Controls purpose of service call
# (must be set, as default value of 0 will produce an error)

int32 signal
# Make an initial action request (must set goal_id).
int32 ACTION_REQUEST = 1
# Halt an on going action request (must set goal_id).
int32 ACTION_HALT = 2
# Resume an on going action request (must set goal_id).
int32 ACTION_RESUME = 3
# Trigger an action once.
int32 ACTION_TICK_ONCE = 4

