#ifndef ANALYSIS_CALLBACK_H
#define ANALYSIS_CALLBACK_H


#include <pin.H>

#include <cassert>
#include <type_traits>

//#include "../tinyformat.h"

template <typename T1>
struct is_well_formed
{
//  template <IARG_TYPE... args>
//  static auto value () -> bool
//  {
//    return (sizeof...(args) == 0);
//  }
};

template <typename Rt>
struct is_well_formed<Rt(void)>
{
  template <IARG_TYPE... args>
  static constexpr auto value () -> bool
  {
    return (sizeof...(args) == 0);
  }
};

template <typename Rt, typename Arg1>
struct is_well_formed<Rt(Arg1)>
{
//  template <IARG_TYPE... args>
  static constexpr auto value () -> bool
  {
//    return (sizeof...(args) == 1);
    return false;
  }

  template <IARG_TYPE arg1/*, IARG_TYPE... args*/>
  static constexpr auto value () -> bool
  {
#if __cplusplus >= 201402L
    switch (arg1) {
    case IARG_ADDRINT:
    case IARG_INST_PTR:
    case IARG_BRANCH_TARGET_ADDR:
    case IARG_MEMORYREAD_EA:
    case IARG_MEMORYREAD2_EA:
    case IARG_MEMORYWRITE_EA:
      return std::is_same<Arg1, ADDRINT>::value;

    case IARG_UINT32:
    case IARG_THREAD_ID:
    case IARG_MEMORYREAD_SIZE:
    case IARG_MEMORYWRITE_SIZE:
      return std::is_same<Arg1, UINT32>::value;

    case IARG_BOOL:
      return std::is_same<Arg1, bool>::value;

    case IARG_REG_REFERENCE:
      return std::is_same<Arg1, PIN_REGISTER*>::value;

    case IARG_REG_CONST_REFERENCE:
      return std::is_same<Arg1, const PIN_REGISTER*>::value;

    case IARG_CONST_CONTEXT:
      return std::is_same<Arg1, const CONTEXT*>::value;

    default:
      return false;
    }
#else
    return (
          (((arg1 == IARG_ADDRINT) || (arg1 == IARG_INST_PTR) || (arg1 == IARG_BRANCH_TARGET_ADDR) ||
            (arg1 == IARG_MEMORYREAD_EA) || (arg1 == IARG_MEMORYREAD2_EA) || (arg1 == IARG_MEMORYWRITE_EA)) &&
           std::is_same<Arg1, ADDRINT>::value) ||

          (((arg1 == IARG_UINT32) || (arg1 == IARG_THREAD_ID) || (arg1 == IARG_MEMORYREAD_SIZE) ||
            (arg1 == IARG_MEMORYWRITE_SIZE)) && std::is_same<Arg1, UINT32>::value) ||

          ((arg1 == IARG_BOOL) && std::is_same<Arg1, bool>::value) ||

          ((arg1 == IARG_REG_REFERENCE) && std::is_same<Arg1, PIN_REGISTER*>::value) ||

          ((arg1 == IARG_REG_CONST_REFERENCE) && std::is_same<Arg1, const PIN_REGISTER*>::value) ||

          ((arg1 == IARG_CONST_CONTEXT) && std::is_same<Arg1, const CONTEXT*>::value)
          );
#endif
  }
};

template <typename Rt, typename Arg1, typename... Args>
struct is_well_formed<Rt(Arg1, Args...)>
{
  template <IARG_TYPE arg1, IARG_TYPE... args>
  static constexpr auto value () -> bool
  {
//#warning "blah"
    return
        is_well_formed<Rt(Arg1)>::template value<arg1>() &&
        is_well_formed<Rt(Args...)>:: template value<args...>();
  }

//  template <>
  static constexpr auto value () -> bool
  {
    return (sizeof...(Args) == 0);
  }

//  template <IARG_TYPE... args>
//  static auto value () -> bool
//  {
//    return (sizeof...(Args) == sizeof...(args));
//  }
};

//template <typename Rt, typename Arg1, typename Arg2>
//struct is_well_formed<Rt(*)(Arg1, Arg2)>
//{
////  enum { value = (sizeof...(args) == 2) };
//  template <IARG_TYPE... args>
//  static constexpr auto value() -> bool
//  {
//    return (sizeof...(args) == 2);
//  }

//  template <IARG_TYPE arg1, IARG_TYPE arg2, IARG_TYPE... args>
//  static constexpr auto value () -> bool
//  {
//    if (is_well_formed<Rt(*)(Arg1)>::value(arg1))
//      return is_well_formed<Rt(*)(Arg2)>::value(arg2);
//    else return false;
//  }

//  template <IARG_TYPE arg1, IARG_TYPE arg2, IARG_TYPE... args>
//  static constexpr auto value () -> bool
//  {
//    return
//      is_well_formed<Rt(*)(Arg1)>::value(arg1) &&
//      is_well_formed<Rt(*)(Arg2)>::value(arg2) &&
//      is_well_formed<Rt(*)(void)>::value();
//  }
//};

//template <typename Rt, typename Arg1, typename Arg2, typename Arg3>
//struct is_well_formed<Rt(*)(Arg1, Arg2, Arg3)>
//{
//  template <IARG_TYPE... args>
//  static constexpr auto value () -> bool
//  {
//    return (sizeof...(args) == 3);
//  }

//  template <IARG_TYPE arg1, IARG_TYPE arg2, IARG_TYPE arg3, IARG_TYPE... args>
//  static constexpr auto value () -> bool
//  {
//    return is_well_formed<Rt(*)(Arg1)>::value(arg1);
//  }
//};

//template <typename Rt, typename T1>
//struct is_well_formed<>
//{
//  enum { value = true }; // default is true
//};
//template <typename Rt, IARG_TYPE arg>
//struct is_well_formed<Rt(*)(void), arg>
//{
//  enum { value = true };
//};

//template <typename Rt, typename T1, IARG_TYPE arg>
//struct is_well_formed<Rt(*)(T1)>
//{
//  enum { value = false };
//};

//template <typename Rt, typename T1, typename T2>
//struct is_well_formed<Rt(*)(void*)>
//{

//};



//template <typename Rt, typename Arg1, IARG_TYPE arg, IARG_TYPE... args>
//struct is_well_formed<Rt(*)(Arg1), arg, args...>
//{
//  //
//}

//template<typename Rt, typename... Args>
//struct is_well_formed
//{
//  enum { value = true }; // now, defaut is true !!!!
//};

//template<typename Rt>
//struct is_well_formed<Rt(*)(void)>
//{
//  enum { value = true };
//};

//template<typename Rt, typename CArg>
//struct is_well_formed<Rt(*)(void), CArg>
//{
//  enum { value = false };
//};

//template<typename Rt, typename Arg1, typename CArg1>
//struct is_well_formed<Rt(*)(Arg1), CArg1>
//{
//  enum { value = std::is_same<CArg1, IARG_TYPE>::value };
//};

//template<typename Rt, typename Arg1, typename CArg1, typename CArg2>
//struct is_well_formed<Rt(*)(Arg1), CArg1, CArg2>
//{
//  enum { value = std::is_same<Arg1, CArg1>::value };
//};

//template

//template<typename F>
//struct argument_tuple;

//template<typename R, typename... Args>
//struct argument_tuple<R(*)(Args...)>
//{
//  template<typename... Arg1s>
//  argument_tuple (Arg1s... args)
//  {
//  }

////  typedef std::tuple<Args...> type;
//  using type = std::tuple<Args...>;

//  enum
//  {
//    typedef
//  };
//};


//template<typename T>
//class instruction_analysis_callback
//{
//private:
//  std::function<T> callback_func_;

//  template<typename Arg>
//  auto check_argument_type_of_idx (uint8_t arg_idx) -> bool
//  {
//    switch (arg_idx) {
//    case 1:
//      return std::is_same<typename boost::function_traits<T>::arg1_type, Arg>::value;

//    case 2:
//      return std::is_same<typename boost::function_traits<T>::arg2_type, Arg>::value;

//    case 3:
//      return std::is_same<typename boost::function_traits<T>::arg3_type, Arg>::value;

//    case 4:
//      return std::is_same<typename boost::function_traits<T>::arg4_type, Arg>::value;

//    case 5:
//      return std::is_same<typename boost::function_traits<T>::arg5_type, Arg>::value;

//    case 6:
//      return std::is_same<typename boost::function_traits<T>::arg6_type, Arg>::value;

//    case 7:
//      return std::is_same<typename boost::function_traits<T>::arg7_type, Arg>::value;

//    default:
//      return false;
//    }
//  }

//  auto check_argument_types (uint8_t arg_idx) -> bool
//  {
//    return true;
//  }

//  template<typename Arg, typename... Args>
//  auto check_argument_types (uint8_t arg_idx, Arg arg, Args... args) -> bool
//  {
//    assert((0 <= arg_idx) && (arg_idx <= 7));

//    if (std::is_same<Arg, IARG_TYPE>::value) {
//      switch (arg) {
//      case IARG_INST_PTR:
//      case IARG_BRANCH_TARGET_ADDR:
//      case IARG_MEMORYREAD_EA:
//      case IARG_MEMORYREAD2_EA:
//      case IARG_MEMORYWRITE_EA:
//        if (check_argument_type_of_idx<ADDRINT>(arg_idx))
//          return check_argument_types(arg_idx + 1, args...);
//        else return false;

//      case IARG_CONST_CONTEXT:
//        if (check_argument_type_of_idx<const CONTEXT*>(arg_idx))
//          return check_argument_types(arg_idx + 1, args...);
//        else return false;

//      case IARG_THREAD_ID:
//      case IARG_MEMORYREAD_SIZE:
//      case IARG_MEMORYWRITE_SIZE:
//        if (check_argument_type_of_idx<UINT32>(arg_idx))
//          return check_argument_types(arg_idx + 1, args...);
//        else return false;

//      case IARG_ADDRINT:
//        if (check_argument_type_of_idx<ADDRINT>(arg_idx + 1))
//          return check_argument_types(arg_idx + 1, args...);
//        else return false;

//      case IARG_UINT32:
//        if (check_argument_type_of_idx<UINT32>(arg_idx + 1))
//          return check_argument_types(arg_idx + 1, args...);
//        else return false;

//      case IARG_BOOL:
//        if (check_argument_type_of_idx<BOOL>(arg_idx + 1))
//          return check_argument_types(arg_idx + 1, args...);
//        else return false;

//      case IARG_REG_REFERENCE:
//        if (check_argument_type_of_idx<PIN_REGISTER*>(arg_idx + 1))
//          return check_argument_types(arg_idx + 1, args...);
//        else return false;

//      case IARG_REG_CONST_REFERENCE:
//        if (check_argument_type_of_idx<const PIN_REGISTER*>(arg_idx + 1))
//          return check_argument_types(arg_idx + 1, args...);
//        else return false;

//      default:
//        return false;
//      }
//    }
//    else if (std::is_same<Arg, decltype(IARG_END)>::value) {
//    }
//  }

//public:
//  instruction_analysis_callback (T callback_func)
//  {
//    static_assert(std::is_function<T>::value,
//                  "instruction analysis callback is not a function");

//    using pin_callback_func_t = std::remove_pointer<AFUNPTR>::type;
//    static_assert(std::is_same<typename boost::function_traits<T>::result_type,
//                  boost::function_traits<pin_callback_func_t>::result_type>::value,
//                  "instruction analysis callback must have the same return type with AFUNPTR");

//    callback_func_ = callback_func;
//  }

//  template<typename... Args>
//  auto insert(INS ins, IPOINT insertion_point, Args... args) -> void
//  {
//    assert(check_argument_types(1, args...));
//    INS_InsertCall(ins, insertion_point, reinterpret_cast<AFUNPTR>(callback_func_. template target<T>()), args...);
//  }
//};


#endif // ANALYSIS_CALLBACK_H

