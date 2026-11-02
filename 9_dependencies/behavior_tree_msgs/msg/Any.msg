# Structure similar to std::any for messages

# Constants for value_type
uint8 BOOL=0
uint8 INT8=1
uint8 UINT8=2
uint8 INT16=3
uint8 UINT16=4
uint8 INT32=5
uint8 UINT32=6
uint8 INT64=7
uint8 UINT64=8
uint8 FLOAT32=9
uint8 FLOAT64=10
uint8 STRING=11
uint8 VECTOR_BOOL=20
uint8 VECTOR_INT8=21
uint8 VECTOR_UINT8=22
uint8 VECTOR_INT16=23
uint8 VECTOR_UINT16=24
uint8 VECTOR_INT32=25
uint8 VECTOR_UINT32=26
uint8 VECTOR_INT64=27
uint8 VECTOR_UINT64=28
uint8 VECTOR_FLOAT32=29
uint8 VECTOR_FLOAT64=30

# The message contents
uint8 type
string value
