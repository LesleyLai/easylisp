#ifndef EASYLISP_CONFIG_HPP
#define EASYLISP_CONFIG_HPP

#define MOV(...)                                                               \
  static_cast<std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)
#define FWD(...) static_cast<decltype(__VA_ARGS__)>(__VA_ARGS__)

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#endif // EASYLISP_CONFIG_HPP
