#include "auth_middleware.hpp"
#include <userver/components/component_context.hpp>
#include <userver/server/http/http_response.hpp>
#include <userver/server/request/request_context.hpp>

namespace blablacar_service {

AuthMiddleware::AuthMiddleware(StoragePgComponent& storage, bool enabled)
    : storage_(storage), enabled_(enabled) {}

void AuthMiddleware::HandleRequest(
    userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
  if (!enabled_) {
    Next(request, context);
    return;
  }

  const auto& auth_header = request.GetHeader("Authorization");
  const std::string bearer_prefix = "Bearer ";

  if (auth_header.starts_with(bearer_prefix)) {
    std::string token = auth_header.substr(bearer_prefix.length());

    auto login = storage_.GetLoginByToken(token);
    if (login) {
      context.SetData("X-User-Login", *login);
      Next(request, context);
      return;
    }
  }

  request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
  request.GetHttpResponse().SetData("{\"error\": \"Unauthorized\"}");
}

AuthMiddlewareFactory::AuthMiddlewareFactory(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpMiddlewareFactoryBase(config, context),
      storage_(context.FindComponent<StoragePgComponent>()) {}

std::unique_ptr<userver::server::middlewares::HttpMiddlewareBase>
AuthMiddlewareFactory::Create(
    const userver::server::handlers::HttpHandlerBase& /*handler*/,
    userver::yaml_config::YamlConfig middleware_config) const {
  bool is_enabled = !middleware_config.IsMissing();

  return std::make_unique<AuthMiddleware>(storage_, is_enabled);
}

void AppendAuthMiddleware(userver::components::ComponentList& component_list) {
  component_list.Append<AuthMiddlewareFactory>();
}

}  // namespace blablacar_service