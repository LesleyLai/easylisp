#include "environment.hpp"

auto Environment::find(const std::string& var) const -> const Value*
{
  auto itr = bindings_.find(var);
  if (itr != bindings_.end()) { return &itr->second; }
  return parent_ ? parent_->find(var) : nullptr;
}


void Environment::add(std::string variable, Value value)
{
  bindings_.insert_or_assign(MOV(variable), MOV(value));
}