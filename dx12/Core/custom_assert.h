#pragma once
#include <assert.h>

#define assert_if_SUCCEEDED(HRESULT, MESSAGE) assert(SUCCEEDED(HRESULT) && MESSAGE)