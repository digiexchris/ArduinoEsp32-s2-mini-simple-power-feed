
#pragma once
#include <memory>
#include <cxxabi.h>
#include <string>
#include <esp_log.h>

#ifdef DEBUG_ABORTS
#define ASSERT(x) if(!(x)) { abort(); }
#define ASSERT_MSG(x, tag, msg) if(!(x)) { ESP_LOGE(tag, "ASSERT FAILED: %s", msg); abort(); }
#else
#define ASSERT(x) if(!(x)) { ESP_LOGE("shared.h", "ASSERT FAILED: %s", #x); }
#define ASSERT_MSG(x, tag, msg) if(!(x)) { ESP_LOGE(tag, "ASSERT FAILED: %s", msg); }
#endif

#define UNUSED(x) (void)(x);

template <class T>
const T &clamp(const T &v, const T &lo, const T &hi)
{
	return (v < lo) ? lo : (hi < v) ? hi
									: v;
}

inline uint32_t mapValueToRange(uint16_t value, uint16_t inMin, uint16_t inMax, uint16_t outMin, uint16_t outMax)
{
	return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
};

static std::string DemangleTypeName(const char *mangledName)
{
	int status = -1;
	std::unique_ptr<char, void (*)(void *)> demangledName(
		abi::__cxa_demangle(mangledName, nullptr, nullptr, &status),
		std::free);
	if (status == 0)
	{
		return std::string(demangledName.get());
	}
	else
	{
		// Demangling failed, return the mangled name
		return std::string(mangledName);
	}
}