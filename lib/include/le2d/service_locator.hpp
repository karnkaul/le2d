#pragma once
#include <klib/assert.hpp>
#include <gsl/pointers>
#include <typeindex>
#include <unordered_map>

namespace le {
class ServiceLocator {
  public:
	template <typename Type>
	void bind(Type* service) {
		KLIB_ASSERT(service != nullptr);
		m_services.insert_or_assign(typeid(Type), service);
	}

	template <typename Type>
	void unbind() {
		m_services.erase(typeid(Type));
	}

	template <typename Type>
	[[nodiscard]] auto contains() const -> bool {
		return find<Type>() != nullptr;
	}

	template <typename Type>
	[[nodiscard]] auto find() const -> Type* {
		auto const it = m_services.find(typeid(Type));
		if (it == m_services.end()) { return nullptr; }
		return static_cast<Type*>(it->second.get());
	}

	template <typename Type>
	[[nodiscard]] auto get() const -> Type& {
		KLIB_ASSERT(contains<Type>());
		return *find<Type>();
	}

	[[nodiscard]] auto service_count() const -> std::size_t { return m_services.size(); }
	void clear() { m_services.clear(); }

  private:
	std::unordered_map<std::type_index, gsl::not_null<void*>> m_services{};
};
} // namespace le
