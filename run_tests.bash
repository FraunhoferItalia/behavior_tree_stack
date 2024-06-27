# Run tests for bt_executor
colcon test --event-handlers console_cohesion+ --packages-select bt_executor

# Run tests for bt_service_server
python3 -m pytest src/bt_service_server/test/test_bt_service_server.py -s

# Run tests for bt_action_server
python3 -m pytest src/bt_action_server/test/test_send_bt_action_server.py -s

# Capture the exit status
test_status=$?

# Check if the tests passed or failed
if [ $test_status -eq 0 ]; then
  echo "All tests passed."
else
  echo "Some tests failed."
fi

# Exit with the same status as pytest
#exit $test_status