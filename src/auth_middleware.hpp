#pragma once

#include <userver/components/component_list.hpp>
#include <userver/server/middlewares/http_middleware_base.hpp>
#include <userver/yaml_config/yaml_config.hpp>
#include "storage.hpp"

namespace blablacar_service {

class AuthMiddleware final
    : public userver::server::middlewares::HttpMiddlewareBase {
 public:
  static constexpr std::string_view kName = "auth-middleware";

  explicit AuthMiddleware(StorageComponent& storage, bool enabled);

  void HandleRequest(
      userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext& context) const override;

 private:
  StorageComponent& storage_;
  bool enabled_;
};

class AuthMiddlewareFactory final
    : public userver::server::middlewares::HttpMiddlewareFactoryBase {
 public:
  static constexpr std::string_view kName = "auth-middleware-factory";

  AuthMiddlewareFactory(const userver::components::ComponentConfig& config,
                        const userver::components::ComponentContext& context);

  std::unique_ptr<userver::server::middlewares::HttpMiddlewareBase> Create(
      const userver::server::handlers::HttpHandlerBase& handler,
      userver::yaml_config::YamlConfig middleware_config) const override;

 private:
  StorageComponent& storage_;
};

void AppendAuthMiddleware(userver::components::ComponentList& component_list);

}  // namespace blablacar_service