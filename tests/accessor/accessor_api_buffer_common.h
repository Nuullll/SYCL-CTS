/*******************************************************************************
//
//  SYCL 2020 Conformance Test Suite
//
//  Copyright:	(c) 2018 by Codeplay Software LTD. All Rights Reserved.
//
*******************************************************************************/
#ifndef SYCL_1_2_1_TESTS_ACCESSOR_ACCESSOR_API_BUFFER_COMMON_H
#define SYCL_1_2_1_TESTS_ACCESSOR_ACCESSOR_API_BUFFER_COMMON_H

#include "../common/common.h"
#include "./../../util/math_helper.h"
#include "accessor_api_common_all.h"
#include "accessor_api_common_buffer_local.h"
#include "accessor_api_utility.h"

namespace {

using namespace sycl_cts;
using namespace accessor_utility;

/** unique dummy_functor per file
 */
template <typename T>
class dummy_accessor_api_buffer {};
template <typename T>
using dummy_functor = ::dummy_functor<dummy_accessor_api_buffer<T>>;

/** explicit pointer type
*/
template <typename T, cl::sycl::access::target target>
struct explicit_pointer;

/** explicit pointer type (specialization for global_buffer)
*/
template <typename T>
struct explicit_pointer<T, cl::sycl::access::target::global_buffer> {
  using type = cl::sycl::global_ptr<T>;
};

/** explicit pointer type (specialization for constant_buffer)
*/
template <typename T>
struct explicit_pointer<T, cl::sycl::access::target::constant_buffer> {
  using type = cl::sycl::constant_ptr<T>;
};

/** explicit pointer type (specialization for host_buffer)
*/
template <typename T>
struct explicit_pointer<T, cl::sycl::access::target::host_buffer> {
  using type = T *;
};

/** explicit pointer alias
 */
template <typename T, cl::sycl::access::target target>
using explicit_pointer_t = typename explicit_pointer<T, target>::type;

////////////////////////////////////////////////////////////////////////////////
// Tests
////////////////////////////////////////////////////////////////////////////////

/** tests buffer accessors methods
*/
template <typename T, int dims, cl::sycl::access::mode mode,
          cl::sycl::access::target target,
          cl::sycl::access::placeholder placeholder =
              cl::sycl::access::placeholder::false_t>
class check_buffer_accessor_api_methods {
 public:
  using acc_t = cl::sycl::accessor<T, dims, mode, target, placeholder>;

  size_t count;
  size_t size;

  void operator()(util::logger &log, cl::sycl::queue &queue,
                  const sycl_range_t<dims> &range,
                  const std::string& typeName) {
#ifdef VERBOSE_LOG
    log_accessor<T, dims, mode, target, placeholder>(
        "check_buffer_accessor_api_methods", typeName, log);
#endif  // VERBOSE_LOG

    auto data = get_buffer_input_data<T>(count, dims);
    buffer_t<T, dims> buffer(data.data(), range);

    // Prepare access range and access offset
    const auto accessRange = range / 2;
    const size_t accessedCount = dims == 0 ? 1 : accessRange.size();
    const size_t accessedSize = accessedCount * sizeof(T);
    auto accessOffset = sycl_id_t<dims>{};
    accessOffset[0] = accessRange[0] / 2;

    check_all_methods(log, queue, accessRange, accessOffset,
                      buffer, accessedSize, accessedCount, typeName,
                      acc_type_tag::get<target, placeholder>());
  }

 private:
  template <typename expectedT, typename returnT>
  void check_acc_return_type(sycl_cts::util::logger& log, returnT returnVal,
                            const std::string& functionName,
                            const std::string& typeName) const {
    accessor_utility::check_acc_return_type<
        expectedT, T, dims, mode, target, placeholder>(
            log, returnVal, functionName, typeName);
  }

  void check_common_methods(util::logger &log, const acc_t &accessor,
                            const size_t accessedSize,
                            const size_t accessedCount,
                            const std::string& typeName) const {
    {
      /** check is_placeholder() method
       */
      auto isPlaceholder = accessor.is_placeholder();
      check_acc_return_type<bool>(log, isPlaceholder, "is_placeholder()",
                                  typeName);
      if (isPlaceholder !=
          (placeholder == cl::sycl::access::placeholder::true_t)) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "accessor does not properly report placeholder status");
      }
    }
    {
      /** check get_count() method
       */
      auto accessorCount = accessor.get_count();
      check_acc_return_type<size_t>(log, accessorCount, "get_count()",
                                    typeName);
      if (accessorCount != accessedCount) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "accessor does not return the correct count");
      }
    }
    {
      /** check get_size() method
       */
      auto accessorSize = accessor.get_size();
      check_acc_return_type<size_t>(log, accessorSize, "get_size()", typeName);
      if (accessorSize != accessedSize) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "accessor does not return the correct size");
      }
    }
    {
      /** check get_pointer() method
       */
      check_acc_return_type<explicit_pointer_t<T, target>>(
          log, accessor.get_pointer(), "get_pointer()", typeName);
    }
  }

  void check_range_offset(util::logger &log,
                          const sycl_range_t<dims> &accessRange,
                          const sycl_id_t<dims> &accessOffset,
                          const acc_t &accessor,
                          const std::string& typeName) const {
    {
      /** check get_range() method
       */
      auto accessorRange = accessor.get_range();
      check_acc_return_type<sycl_range_t<dims>>(log, accessor.get_range(),
                                                "get_range()", typeName);
      if (accessorRange != accessRange) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "accessor does not return the correct range");
      }
    }
    {
      /** check get_offset() method
       */
      auto accessorOffset = accessor.get_offset();
      check_acc_return_type<sycl_id_t<dims>>(log, accessor.get_offset(),
                                             "get_offset()", typeName);
      if (accessorOffset != accessOffset) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "accessor does not return the correct offset");
      }
    }
  }

  /**
   * @brief Checks member functions where (dims != 0)
   * @param log The logger object
   * @param accessRange The range used on construction
   * @param accessOffset The offset used on construction
   * @param accessor Accessor under test
   */
  void check_methods(util::logger &log, const sycl_range_t<dims> &accessRange,
                     const sycl_id_t<dims> &accessOffset, const acc_t &accessor,
                     const size_t accessedSize, const size_t accessedCount,
                     const std::string& typeName,
                     generic_dim_tag) const {
    check_common_methods(log, accessor, accessedSize, accessedCount, typeName);
    check_range_offset(log, accessRange, accessOffset, accessor, typeName);
  }

  /**
   * @brief Checks member functions where (dims == 0)
   * @param log The logger object
   * @param accessor Accessor under test
   */
  void check_methods(util::logger &log,
                     const sycl_range_t<dims> & /*accessRange*/,
                     const sycl_id_t<dims> & /*accessOffset*/,
                     const acc_t &accessor,
                     const size_t accessedSize,
                     const size_t accessedCount,
                     const std::string& typeName,
                     zero_dim_tag) const {
    check_common_methods(log, accessor, accessedSize, accessedCount, typeName);
    // Zero-dim accessors do not provide get_range() and get_offset()
  }

  /**
   * @brief Checks member functions of accessors that can be used in kernels
   * @param log The logger object
   * @param queue SYCL queue where a kernel will be executed
   * @param accessRange The range of the accessor
   * @param accessOffset The offset of the accessor
   * @param buffer SYCL buffer used for constructing the accessor
   */
  void check_all_methods(util::logger &log, cl::sycl::queue &queue,
                         const sycl_range_t<dims> &accessRange,
                         const sycl_id_t<dims> &accessOffset,
                         buffer_t<T, dims> &buffer,
                         const size_t accessedSize,
                         const size_t accessedCount,
                         const std::string& typeName,
                         acc_type_tag::generic) const {
    queue.submit([&](cl::sycl::handler &cgh) {
      auto acc = make_accessor_generic<dims, mode, target, placeholder>(
          buffer, &accessRange, &accessOffset, &cgh);
      check_methods(log, accessRange, accessOffset, acc, accessedSize,
          accessedCount, typeName, is_zero_dim<dims>{});
      cgh.single_task(dummy_functor<T>{});
    });
  }

  /**
   * @brief Checks member functions of host accessors
   * @param log The logger object
   * @param accessRange The range of the accessor
   * @param accessOffset The offset of the accessor
   * @param buffer SYCL buffer used for constructing the accessor
   */
  void check_all_methods(util::logger &log, cl::sycl::queue & /*queue*/,
                         const sycl_range_t<dims> &accessRange,
                         const sycl_id_t<dims> &accessOffset,
                         buffer_t<T, dims> &buffer, const size_t accessedSize,
                         size_t accessedCount, const std::string& typeName,
                         acc_type_tag::host) const {
    auto acc = make_accessor_generic<dims, mode, target>(
        buffer, &accessRange, &accessOffset, nullptr);
    check_methods(log, accessRange, accessOffset, acc,accessedSize,
        accessedCount, typeName, is_zero_dim<dims>{});
  }
};

template <typename T, int dims, cl::sycl::access::mode mode,
          cl::sycl::access::target target,
          cl::sycl::access::placeholder placeholder =
              cl::sycl::access::placeholder::false_t>
class check_buffer_accessor_api {
  using acc_t = cl::sycl::accessor<T, dims, mode, target, placeholder>;

 public:
  size_t count;
  size_t size;

  /** tests buffer accessors reads
  */
  void operator()(util::logger &log, cl::sycl::queue &queue,
                  sycl_range_t<dims> range, const std::string& typeName,
                  acc_mode_tag::read_only) {
#ifdef VERBOSE_LOG
    log_accessor<T, dims, mode, target, placeholder>(
        "check_buffer_accessor_api::reads", typeName, log);
#endif  // VERBOSE_LOG

    auto dataIdSyntax = get_buffer_input_data<T>(count, dims);
    auto dataMultiDimSyntax = get_buffer_input_data<T>(count, dims);
    auto errors = get_error_data(2);

    {
      buffer_t<T, dims> bufIdSyntax(dataIdSyntax.data(), range);
      buffer_t<T, dims> bufMultiDimSyntax(dataMultiDimSyntax.data(), range);
      error_buffer_t errorBuffer(errors.data(),
                                 cl::sycl::range<1>(errors.size()));

      check_command_group_read_only(queue, bufIdSyntax, bufMultiDimSyntax,
                                    errorBuffer, range,
                                    acc_type_tag::get<target, placeholder>());
    }

    if (dims == 0) {
      if (errors[0] != 0) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "operator dataT&() did not read from the correct index");
      }
    } else {
      if (errors[0] != 0) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "operator[id<N>] did not read from the correct index");
      }
      if (errors[1] != 0) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "operator[size_t][size_t][size_t] did not read from the correct "
            "index");
      }
    }
  }

 private:
  /**
   * @brief Checks reading from an accessor using subscript operators.
   *        Executed only for accessor that can be used in kernels.
   * @param queue SYCL queue where a kernel will be executed
   * @param bufIdSyntax SYCL buffer used for testing reading an accessor
   *        by passing an ID to the subscript operator
   * @param bufMultiDimSyntax SYCL buffer used for testing reading an accessor
   *        by using the multidimensional subscript operators
   * @param errorBuffer Buffer where errors will be stored
   * @param range The range of the data buffers
   */
  void check_command_group_read_only(cl::sycl::queue &queue,
                                     buffer_t<T, dims> &bufIdSyntax,
                                     buffer_t<T, dims> &bufMultiDimSyntax,
                                     error_buffer_t &errorBuffer,
                                     sycl_range_t<dims> range,
                                     acc_type_tag::generic) {
    queue.submit([&](cl::sycl::handler &handler) {
      auto accIdSyntax =
          make_accessor<T, dims, mode, target>(bufIdSyntax, handler);
      auto accMultiDimSyntax =
          make_accessor<T, dims, mode, target>(bufMultiDimSyntax, handler);
      static constexpr auto errorTarget =
          cl::sycl::access::target::global_buffer;
      auto errorAccessor =
          make_accessor<int, 1, errorMode, errorTarget>(errorBuffer, handler);

      handler.parallel_for(
          range,
          buffer_accessor_api_r<T, dims, mode, target, errorTarget>(
              size, accIdSyntax, accMultiDimSyntax, errorAccessor, range));
    });
  }

  /**
   * @brief Checks reading from a host accessor using subscript operators
   * @param bufIdSyntax SYCL buffer used for testing reading an accessor
   *        by passing an ID to the subscript operator
   * @param bufMultiDimSyntax SYCL buffer used for testing reading an accessor
   *        by using the multidimensional subscript operators
   * @param errorBuffer Buffer where errors will be stored
   * @param range The range of the data buffers
   */
  void check_command_group_read_only(cl::sycl::queue & /*queue*/,
                                     buffer_t<T, dims> &bufIdSyntax,
                                     buffer_t<T, dims> &bufMultiDimSyntax,
                                     error_buffer_t &errorBuffer,
                                     sycl_range_t<dims> range,
                                     acc_type_tag::host) {
    auto accIdSyntax = make_accessor<T, dims, mode, target>(bufIdSyntax);
    auto accMultiDimSyntax =
        make_accessor<T, dims, mode, target>(bufMultiDimSyntax);
    static constexpr auto errorTarget = cl::sycl::access::target::host_buffer;
    auto errorAccessor =
        make_accessor<int, 1, errorMode, errorTarget>(errorBuffer);

    /** check buffer accessor subscript operators for reads
    */
    auto idList = create_id_list<data_dim<dims>::value>(range);
    for (auto id : idList) {
      buffer_accessor_api_r<T, dims, mode, target, errorTarget>(
          size, accIdSyntax, accMultiDimSyntax, errorAccessor, range)(id);
    }
  }

  /**
   * @brief Checks reading from a placeholder accessor
   *        using subscript operators
   * @param queue SYCL queue where a kernel will be executed
   * @param bufIdSyntax SYCL buffer used for testing reading an accessor
   *        by passing an ID to the subscript operator
   * @param bufMultiDimSyntax SYCL buffer used for testing reading an accessor
   *        by using the multidimensional subscript operators
   * @param errorBuffer Buffer where errors will be stored
   * @param range The range of the data buffers
   */
  void check_command_group_read_only(cl::sycl::queue &queue,
                                     buffer_t<T, dims> &bufIdSyntax,
                                     buffer_t<T, dims> &bufMultiDimSyntax,
                                     error_buffer_t &errorBuffer,
                                     sycl_range_t<dims> range,
                                     acc_type_tag::placeholder) {
    auto a1 =
        cl::sycl::accessor<T, dims, mode, target,
                           cl::sycl::access::placeholder::true_t>(bufIdSyntax);
    auto a2 = cl::sycl::accessor<T, dims, mode, target,
                                 cl::sycl::access::placeholder::true_t>(
        bufMultiDimSyntax);

    queue.submit([&](cl::sycl::handler &h) {
      h.require(a1);
      h.require(a2);

      static constexpr auto errorTarget =
          cl::sycl::access::target::global_buffer;

      auto errorAccessor =
          make_accessor<int, 1, errorMode, errorTarget>(errorBuffer, h);

      const auto accSize = size;

      auto reader = buffer_accessor_api_r<T, dims, mode, target, errorTarget,
                                          placeholder>{accSize, a1, a2,
                                                       errorAccessor, range};

      h.parallel_for(range, reader);
    });
  }

 public:
  /** tests buffer accessors writes
  */
  void operator()(util::logger &log, cl::sycl::queue &queue,
                  sycl_range_t<dims> range, const std::string& typeName,
                  acc_mode_tag::write_only) {
#ifdef VERBOSE_LOG
    log_accessor<T, dims, mode, target, placeholder>(
        "check_buffer_accessor_api::writes", typeName, log);
#endif  // VERBOSE_LOG

    static constexpr bool useIndexes = false;
    auto dataIdSyntax = get_buffer_input_data<T>(count, dims, useIndexes);
    auto dataMultiDimSyntax = get_buffer_input_data<T>(count, dims, useIndexes);

    {
      buffer_t<T, dims> bufIdSyntax(dataIdSyntax.data(), range);
      buffer_t<T, dims> bufMultiDimSyntax(dataMultiDimSyntax.data(), range);

      check_command_group_writes(queue, bufIdSyntax, bufMultiDimSyntax, range,
                                 acc_type_tag::get<target, placeholder>());
    }

    if (dims == 0) {
      const auto expected = get_zero_dim_buffer_value<T>();
      if (!check_elems_equal(dataMultiDimSyntax[0], expected)) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "operator dataT&() did not write to the correct index");
      }
    } else {
      if (!check_linear_index(log, dataIdSyntax.data(), count)) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "operator[id<N>] did not assign to the correct index");
      }
      if (!check_linear_index(log, dataMultiDimSyntax.data(), count)) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "operator[size_t][size_t][size_t] did not assign to the correct "
            "index");
      }
    }
  }

 private:
  /**
   * @brief Checks writing to an accessor using subscript operators.
   *        Executed only for accessor that can be used in kernels.
   * @param queue SYCL queue where a kernel will be executed
   * @param bufIdSyntax SYCL buffer used for testing writing to an accessor
   *        by passing an ID to the subscript operator
   * @param bufMultiDimSyntax SYCL buffer used for testing writing to an
   *        accessor by using the multidimensional subscript operators
   * @param range The range of the data buffers
   */
  void check_command_group_writes(cl::sycl::queue &queue,
                                  buffer_t<T, dims> &bufIdSyntax,
                                  buffer_t<T, dims> &bufMultiDimSyntax,
                                  sycl_range_t<dims> range,
                                  acc_type_tag::generic) {
    queue.submit([&](cl::sycl::handler &handler) {
      auto accIdSyntax = make_accessor<T, dims, mode, target, placeholder>(
          bufIdSyntax, handler);
      auto accMultiDimSyntax =
          make_accessor<T, dims, mode, target, placeholder>(bufMultiDimSyntax,
                                                            handler);

      /** check buffer accessor subscript operators for writes
      */
      handler.parallel_for(
          range, buffer_accessor_api_w<T, dims, mode, target, placeholder>(
                     size, accIdSyntax, accMultiDimSyntax, range));
    });
  }

  /**
   * @brief Checks writing to a host accessor using subscript operators
   * @param bufIdSyntax SYCL buffer used for testing writing to an accessor
   *        by passing an ID to the subscript operator
   * @param bufMultiDimSyntax SYCL buffer used for testing writing to an
   *        accessor by using the multidimensional subscript operators
   * @param range The range of the data buffers
   */
  void check_command_group_writes(cl::sycl::queue & /*queue*/,
                                  buffer_t<T, dims> &bufIdSyntax,
                                  buffer_t<T, dims> &bufMultiDimSyntax,
                                  sycl_range_t<dims> range,
                                  acc_type_tag::host) {
    auto accIdSyntax = make_accessor<T, dims, mode, target>(bufIdSyntax);
    auto accMultiDimSyntax =
        make_accessor<T, dims, mode, target>(bufMultiDimSyntax);

    /** check buffer accessor subscript operators for writes
    */
    auto idList = create_id_list<data_dim<dims>::value>(range);
    for (auto id : idList) {
      buffer_accessor_api_w<T, dims, mode, target>(
          size, accIdSyntax, accMultiDimSyntax, range)(id);
    }
  }

  /**
   * @brief Checks writing to a placeholder accessor using subscript operators
   * @param queue SYCL queue where a kernel will be executed
   * @param bufIdSyntax SYCL buffer used for testing writing to an accessor
   *        by passing an ID to the subscript operator
   * @param bufMultiDimSyntax SYCL buffer used for testing writing to an
   *        accessor by using the multidimensional subscript operators
   * @param range The range of the data buffers
   */
  void check_command_group_writes(cl::sycl::queue &queue,
                                  buffer_t<T, dims> &bufIdSyntax,
                                  buffer_t<T, dims> &bufMultiDimSyntax,
                                  sycl_range_t<dims> range,
                                  acc_type_tag::placeholder) {
    auto a1 =
        cl::sycl::accessor<T, dims, mode, target,
                           cl::sycl::access::placeholder::true_t>(bufIdSyntax);
    auto a2 = cl::sycl::accessor<T, dims, mode, target,
                                 cl::sycl::access::placeholder::true_t>(
        bufMultiDimSyntax);

    queue.submit([&](cl::sycl::handler &h) {
      h.require(a1);
      h.require(a2);
      auto writer =
          buffer_accessor_api_w<T, dims, mode, target>{size, a1, a2, range};

      h.parallel_for(range, writer);
    });
  }

 public:
  /** tests buffer accessors reads and writes
  */
  void operator()(util::logger &log, cl::sycl::queue &queue,
                  sycl_range_t<dims> range, const std::string& typeName,
                  acc_mode_tag::generic) {
#ifdef VERBOSE_LOG
    log_accessor<T, dims, mode, target, placeholder>(
        "check_buffer_accessor_api::reads_and_writes", typeName, log);
#endif  // VERBOSE_LOG

    // In case of dims == 0, there will be a read from dataIdSyntax
    // and a write to dataMultiDimSyntax
    static constexpr bool useIndexesWrite = (dims > 0);
    auto dataIdSyntax = get_buffer_input_data<T>(count, dims);
    auto dataMultiDimSyntax =
        get_buffer_input_data<T>(count, dims, useIndexesWrite);

    static constexpr bool isHostBuffer =
        (target == cl::sycl::access::target::host_buffer);
    auto errors = get_error_data(isHostBuffer ? 2 : 4);

    {
      buffer_t<T, dims> bufIdSyntax(dataIdSyntax.data(), range);
      buffer_t<T, dims> bufMultiDimSyntax(dataMultiDimSyntax.data(), range);
      error_buffer_t errorBuffer(errors.data(),
                                 cl::sycl::range<1>(errors.size()));

      check_command_group_reads_writes(
          queue, bufIdSyntax, bufMultiDimSyntax, errorBuffer, range,
          acc_type_tag::get<target, placeholder>());
    }

    if (dims == 0) {
      if ((mode != cl::sycl::access::mode::discard_read_write) &&
          (errors[0] != 0)) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "operator dataT&() did not read from the correct index");
      }
      const auto expected = get_zero_dim_buffer_value<T>();
      if (!check_elems_equal(dataMultiDimSyntax[0], expected)) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "operator dataT&() did not write to the correct index");
      }
    } else {
      if (mode != cl::sycl::access::mode::discard_read_write) {
        if (errors[0] != 0) {
          fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "operator[id<N>] did not read from the correct index");
        }
        if (errors[1] != 0) {
          fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
              "operator[size_t][size_t][size_t] did not read from the "
              "correct index");
        }
      }
      if (!check_linear_index(log, dataIdSyntax.data(), count, 2)) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "operator[id<N>] did not assign to the correct index");
      }
      if (!check_linear_index(log, dataMultiDimSyntax.data(), count, 2)) {
        fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
            "operator dataT&() did not assign to the "
            "correct index");
      }
      if (!isHostBuffer) {
        if (errors[2] != 0) {
          fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
              "operator[id<N>] did not write to the correct index");
        }
        if (errors[3] != 0) {
          fail_for_accessor<T, dims, mode, target, placeholder>(log, typeName,
              "operator[size_t][size_t][size_t] did not write to the correct "
              "index");
        }
      }
    }
  }

 private:
  /**
   * @brief Checks reading from and writing to an accessor
   *        using subscript operators.
   *        Executed only for accessor that can be used in kernels.
   * @param queue SYCL queue where a kernel will be executed
   * @param bufIdSyntax SYCL buffer used for testing writing to an accessor
   *        by passing an ID to the subscript operator
   * @param bufMultiDimSyntax SYCL buffer used for testing writing to an
   *        accessor by using the multidimensional subscript operators
   * @param errorBuffer Buffer where errors will be stored
   * @param range The range of the data buffers
   */
  void check_command_group_reads_writes(cl::sycl::queue &queue,
                                        buffer_t<T, dims> &bufIdSyntax,
                                        buffer_t<T, dims> &bufMultiDimSyntax,
                                        error_buffer_t &errorBuffer,
                                        sycl_range_t<dims> range,
                                        acc_type_tag::generic) {
    queue.submit([&](cl::sycl::handler &handler) {
      auto accIdSyntax = make_accessor<T, dims, mode, target, placeholder>(
          bufIdSyntax, handler);
      auto accMultiDimSyntax =
          make_accessor<T, dims, mode, target, placeholder>(bufMultiDimSyntax,
                                                            handler);
      static constexpr auto errorTarget =
          cl::sycl::access::target::global_buffer;
      auto errorAccessor =
          make_accessor<int, 1, errorMode, errorTarget>(errorBuffer, handler);

      /** check buffer accessor subscript operators for reads and writes
      */
      handler.parallel_for(
          range,
          buffer_accessor_api_rw<T, dims, mode, target, errorTarget>(
              size, accIdSyntax, accMultiDimSyntax, errorAccessor, range));
    });
  }

  /**
   * @brief Checks reading from and writing to a host accessor
   *        using subscript operators
   * @param queue SYCL queue where a kernel will be executed
   * @param bufIdSyntax SYCL buffer used for testing writing to an accessor
   *        by passing an ID to the subscript operator
   * @param bufMultiDimSyntax SYCL buffer used for testing writing to an
   *        accessor by using the multidimensional subscript operators
   * @param errorBuffer Buffer where errors will be stored
   * @param range The range of the data buffers
   */
  void check_command_group_reads_writes(cl::sycl::queue & /*queue*/,
                                        buffer_t<T, dims> &bufIdSyntax,
                                        buffer_t<T, dims> &bufMultiDimSyntax,
                                        error_buffer_t &errorBuffer,
                                        sycl_range_t<dims> range,
                                        acc_type_tag::host) {
    auto accIdSyntax = make_accessor<T, dims, mode, target>(bufIdSyntax);
    auto accMultiDimSyntax =
        make_accessor<T, dims, mode, target>(bufMultiDimSyntax);
    auto errorAccessor =
        make_accessor<int, 1, errorMode, cl::sycl::access::target::host_buffer>(
            errorBuffer);

    /** check buffer accessor subscript operators for reads and writes
    */
    auto idList = create_id_list<data_dim<dims>::value>(range);
    for (auto id : idList) {
      buffer_accessor_api_rw<T, dims, mode, target,
                             cl::sycl::access::target::host_buffer>(
          size, accIdSyntax, accMultiDimSyntax, errorAccessor, range)(id);
    }
  }

  /**
   * @brief Checks reading from and writing to a placeholder accessor
   *        using subscript operators
   * @param queue SYCL queue where a kernel will be executed
   * @param bufIdSyntax SYCL buffer used for testing writing to an accessor
   *        by passing an ID to the subscript operator
   * @param bufMultiDimSyntax SYCL buffer used for testing writing to an
   *        accessor by using the multidimensional subscript operators
   * @param errorBuffer Buffer where errors will be stored
   * @param range The range of the data buffers
   */
  void check_command_group_reads_writes(cl::sycl::queue &queue,
                                        buffer_t<T, dims> &bufIdSyntax,
                                        buffer_t<T, dims> &bufMultiDimSyntax,
                                        error_buffer_t &errorBuffer,
                                        sycl_range_t<dims> range,
                                        acc_type_tag::placeholder) {
    auto a1 =
        cl::sycl::accessor<T, dims, mode, target,
                           cl::sycl::access::placeholder::true_t>(bufIdSyntax);
    auto a2 = cl::sycl::accessor<T, dims, mode, target,
                                 cl::sycl::access::placeholder::true_t>(
        bufMultiDimSyntax);

    queue.submit([&](cl::sycl::handler &h) {
      h.require(a1);
      h.require(a2);
      static constexpr auto errorTarget =
          cl::sycl::access::target::global_buffer;

      auto errorAccessor =
          make_accessor<int, 1, errorMode, errorTarget>(errorBuffer, h);

      const auto accSize = size;

      auto reader_writer = buffer_accessor_api_rw<T, dims, mode, target,
                                                  errorTarget, placeholder>{
          accSize, a1, a2, errorAccessor, range};

      h.parallel_for(range, reader_writer);
    });
  }
};

////////////////////////////////////////////////////////////////////////////////
// Enable tests for all combinations
////////////////////////////////////////////////////////////////////////////////

/** tests buffer accessors with different modes
*/
template <typename T, int dims, cl::sycl::access::mode mode,
          cl::sycl::access::target target,
          cl::sycl::access::placeholder placeholder =
              cl::sycl::access::placeholder::false_t>
void check_buffer_accessor_api_mode(util::logger &log, size_t count,
                                    size_t size, cl::sycl::queue &queue,
                                    sycl_range_t<dims> range,
                                    const std::string& typeName) {
#ifdef VERBOSE_LOG
  log_accessor<T, dims, mode, target, placeholder>("", typeName, log);
#endif

  /** check buffer accessor members
   */
  check_accessor_members<T, dims, mode, target, placeholder>(log, typeName);

  /** check buffer accessor methods
   */
  check_buffer_accessor_api_methods<T, dims, mode, target, placeholder>{
      count, size}(log, queue, range, typeName);

  /** check buffer accessor subscript operators
   */
  check_buffer_accessor_api<T, dims, mode, target, placeholder>{count, size}(
      log, queue, range, typeName, acc_mode_tag::get<mode>());
}

template <typename T, int dims, cl::sycl::access::target target,
          cl::sycl::access::placeholder placeholder =
              cl::sycl::access::placeholder::false_t>
void check_buffer_accessor_api_target(util::logger &log, size_t count,
                                      size_t size, cl::sycl::queue &queue,
                                      sycl_range_t<dims> range,
                                      const std::string& typeName,
                                      acc_target_tag::generic) {
  using cl::sycl::access::mode;

  /** check buffer accessor api for read
  */
  check_buffer_accessor_api_mode<T, dims, mode::read, target, placeholder>(
      log, count, size, queue, range, typeName);

  /** check buffer accessor api for read_write
  */
  check_buffer_accessor_api_mode<T, dims, mode::read_write, target,
                                 placeholder>(log, count, size, queue, range,
                                              typeName);

  /** check buffer accessor api for write
  */
  check_buffer_accessor_api_mode<T, dims, mode::write, target>(log, count, size,
                                                               queue, range,
                                                               typeName);

  /** check buffer accessor api for discard_write
  */
  check_buffer_accessor_api_mode<T, dims, mode::discard_write, target>(
      log, count, size, queue, range, typeName);

  /** check buffer accessor api for discard_read_write
  */
  check_buffer_accessor_api_mode<T, dims, mode::discard_read_write, target>(
      log, count, size, queue, range, typeName);
}

template <typename T, int dims, cl::sycl::access::target target,
          cl::sycl::access::placeholder placeholder =
              cl::sycl::access::placeholder::false_t>
void check_buffer_accessor_api_target(util::logger &log, size_t count,
                                      size_t size, cl::sycl::queue &queue,
                                      sycl_range_t<dims> range,
                                      const std::string& typeName,
                                      acc_target_tag::constant) {
  using cl::sycl::access::mode;

  /** check buffer accessor api for read
  */
  check_buffer_accessor_api_mode<T, dims, mode::read, target, placeholder>(
      log, count, size, queue, range, typeName);
}

template <typename T, int dims, cl::sycl::access::target target,
          cl::sycl::access::placeholder placeholder =
              cl::sycl::access::placeholder::false_t>
void check_buffer_accessor_api_target(util::logger &log, size_t count,
                                      size_t size, cl::sycl::queue &queue,
                                      sycl_range_t<dims> range,
                                      const std::string& typeName,
                                      acc_target_tag::host) {
  using cl::sycl::access::mode;

  /** check buffer accessor api for read
  */
  check_buffer_accessor_api_mode<T, dims, mode::read, target, placeholder>(
      log, count, size, queue, range, typeName);

  /** check buffer accessor api for read_write
  */
  check_buffer_accessor_api_mode<T, dims, mode::read_write, target,
                                 placeholder>(log, count, size, queue, range,
                                              typeName);

  /** check buffer accessor api for write
  */
  check_buffer_accessor_api_mode<T, dims, mode::write, target>(log, count, size,
                                                               queue, range,
                                                               typeName);
}

/** tests buffer accessors with different targets
*/
template <typename T, int dims, cl::sycl::access::target target,
          cl::sycl::access::placeholder placeholder =
              cl::sycl::access::placeholder::false_t>
void check_buffer_accessor_api_target_wrapper(util::logger &log, size_t count,
                                      size_t size, cl::sycl::queue &queue,
                                      sycl_range_t<dims> range,
                                      const std::string& typeName) {
  check_buffer_accessor_api_target<T, dims, target, placeholder>(
      log, count, size, queue, range, typeName, acc_target_tag::get<target>());
}

/** tests buffer accessors with different placeholder values
*/
template <typename T, int dims, cl::sycl::access::target target>
void check_buffer_accessor_api_placeholder(util::logger &log, size_t count,
                                           size_t size, cl::sycl::queue &queue,
                                           sycl_range_t<dims> range,
                                           const std::string& typeName) {
  check_buffer_accessor_api_target_wrapper<T, dims, target,
                                   cl::sycl::access::placeholder::false_t>(
      log, count, size, queue, range, typeName);

  check_buffer_accessor_api_target_wrapper<T, dims, target,
                                   cl::sycl::access::placeholder::true_t>(
      log, count, size, queue, range, typeName);
}

/** tests buffer accessors with different dimensions
*/
template <typename T, int dims>
void check_buffer_accessor_api_dim(util::logger &log, size_t count, size_t size,
                                   cl::sycl::queue &queue,
                                   sycl_range_t<dims> range,
                                   const std::string& typeName) {
  /** check buffer accessor api for global_buffer
  */
  check_buffer_accessor_api_placeholder<
      T, dims, cl::sycl::access::target::global_buffer>(log, count, size, queue,
                                                        range, typeName);

  /** check buffer accessor api for constant_buffer
  */
  check_buffer_accessor_api_placeholder<
      T, dims, cl::sycl::access::target::constant_buffer>(log, count, size,
                                                          queue, range,
                                                          typeName);

  /** check buffer accessor api for host_buffer
  */
  check_buffer_accessor_api_target_wrapper<T, dims,
                                   cl::sycl::access::target::host_buffer>(
      log, count, size, queue, range, typeName);
}

/** tests buffer accessors with different types
*/
template <typename T>
class check_buffer_accessor_api_type {
  static constexpr auto count = 8;
  static constexpr auto size = count * sizeof(T);

 public:
  void operator()(util::logger &log, cl::sycl::queue &queue,
                  const std::string& typeName) {
    /** check buffer accessor api for 0 dimension
     */
    cl::sycl::range<1> range0d(count);
    check_buffer_accessor_api_dim<T, 0>(log, count, size, queue, range0d,
                                        typeName);

    /** check buffer accessor api for 1 dimension
     */
    cl::sycl::range<1> range1d(range0d);
    check_buffer_accessor_api_dim<T, 1>(log, count, size, queue, range1d,
                                        typeName);

    /** check buffer accessor api for 2 dimension
     */
    cl::sycl::range<2> range2d(count / 4, 4);
    check_buffer_accessor_api_dim<T, 2>(log, count, size, queue, range2d,
                                        typeName);

    /** check buffer accessor api for 3 dimension
     */
    cl::sycl::range<3> range3d(count / 8, 4, 2);
    check_buffer_accessor_api_dim<T, 3>(log, count, size, queue, range3d,
                                        typeName);
  }
};

}  // namespace

#endif  // SYCL_1_2_1_TESTS_ACCESSOR_ACCESSOR_API_BUFFER_COMMON_H
