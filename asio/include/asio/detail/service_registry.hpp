//
// detail/service_registry.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_SERVICE_REGISTRY_HPP
#define ASIO_DETAIL_SERVICE_REGISTRY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include <typeinfo>
#include "asio/detail/mutex.hpp"
#include "asio/detail/noncopyable.hpp"
#include "asio/detail/type_traits.hpp"
#include "asio/execution_context.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

class io_context;

namespace detail {

template <typename T>
class typeid_wrapper {};

class service_registry
  : private noncopyable
{
public:
  // Constructor.
  ASIO_DECL service_registry(execution_context& owner);

  // Destructor.
  ASIO_DECL ~service_registry();

  // Shutdown all services.
  ASIO_DECL void shutdown_services();

  // Destroy all services.
  ASIO_DECL void destroy_services();

  // Notify all services of a fork event.
  ASIO_DECL void notify_fork(execution_context::fork_event fork_ev);

  // Get the service object corresponding to the specified service type. Will
  // create a new service object automatically if no such object already
  // exists. Ownership of the service object is not transferred to the caller.
  template <typename Service>
  Service& use_service();

  // Get the service object corresponding to the specified service type. Will
  // create a new service object automatically if no such object already
  // exists. Ownership of the service object is not transferred to the caller.
  // This overload is used for backwards compatibility with services that
  // inherit from io_context::service.
  template <typename Service>
  Service& use_service(io_context& owner);

  // Create and add a service object.
  template <typename Service, typename... Args>
  Service& make_service(Args&&... args);

  // Add a service object. Throws on error, in which case ownership of the
  // object is retained by the caller.
  template <typename Service>
  void add_service(Service* new_service);

  // Check whether a service object of the specified type already exists.
  template <typename Service>
  bool has_service() const;

private:
  // Initialise a service's key when the key_type typedef is not available.
  template <typename Service>
  static void init_key(execution_context::service::key& key, ...);

#if !defined(ASIO_NO_TYPEID)
  // Initialise a service's key when the key_type typedef is available.
  template <typename Service>
  static void init_key(execution_context::service::key& key,
      enable_if_t<is_base_of<typename Service::key_type, Service>::value>*);
#endif // !defined(ASIO_NO_TYPEID)

  // Initialise a service's key based on its id.
  ASIO_DECL static void init_key_from_id(
      execution_context::service::key& key,
      const execution_context::id& id);

#if !defined(ASIO_NO_TYPEID)
  // Initialise a service's key based on its id.
  template <typename Service>
  static void init_key_from_id(execution_context::service::key& key,
      const service_id<Service>& /*id*/);
#endif // !defined(ASIO_NO_TYPEID)

  // Check if a service matches the given id.
  ASIO_DECL static bool keys_match(
      const execution_context::service::key& key1,
      const execution_context::service::key& key2);

  // The type of a factory function used for creating a service instance.
  typedef execution_context::service*(*factory_type)(execution_context&, void*);

  // Factory function for creating a service instance.
  template <typename Service, typename Owner, typename... Args>
  static execution_context::service* create(
      execution_context& context, void* owner, Args&&... args);

  // Helper function to destroy an allocated service instance.
  template <typename Service>
  static void destroy_allocated(execution_context::service* service);

  // Helper function to destroy an added service instance.
  ASIO_DECL static void destroy_added(
      execution_context::service* service);

  // Helper class to manage service pointers.
  struct auto_service_ptr;
  friend struct auto_service_ptr;
  struct auto_service_ptr
  {
    execution_context::service* ptr_;
    ASIO_DECL ~auto_service_ptr();
  };

  // Get the service object corresponding to the specified service key. Will
  // create a new service object automatically if no such object already
  // exists. Ownership of the service object is not transferred to the caller.
  ASIO_DECL execution_context::service* do_use_service(
      const execution_context::service::key& key,
      factory_type factory, void* owner);

  // Add a service object. Throws on error, in which case ownership of the
  // object is retained by the caller.
  ASIO_DECL void do_add_service(
      const execution_context::service::key& key,
      execution_context::service* new_service);

  // Check whether a service object with the specified key already exists.
  ASIO_DECL bool do_has_service(
      const execution_context::service::key& key) const;

  // Mutex to protect access to internal data.
  mutable asio::detail::mutex mutex_;

  // The owner of this service registry and the services it contains.
  execution_context& owner_;

  // The first service in the list of contained services.
  execution_context::service* first_service_;
};

} // namespace detail
} // namespace asio

#include "asio/detail/pop_options.hpp"

#include "asio/detail/impl/service_registry.hpp"
#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/impl/service_registry.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_DETAIL_SERVICE_REGISTRY_HPP
