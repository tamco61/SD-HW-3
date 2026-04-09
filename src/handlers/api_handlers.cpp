#include "api_handlers.hpp"
#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <utility>
#include <stdexcept>

namespace blablacar_service {

namespace {
using userver::formats::json::FromString;
using userver::formats::json::ToString;
using userver::formats::json::ValueBuilder;
using userver::server::http::HttpStatus;

template <typename Func>
std::string SafeHandle(const userver::server::http::HttpRequest& request, Func&& f) {
  try {
    return f();
  } catch (const userver::formats::json::Exception& e) {
    request.SetResponseStatus(HttpStatus::kBadRequest);
    return "{\"error\": \"Invalid JSON or missing fields\"}";
  } catch (const std::invalid_argument& e) {
    request.SetResponseStatus(HttpStatus::kBadRequest);
    return "{\"error\": \"Bad request\"}";
  } catch (const std::exception& e) {
    request.SetResponseStatus(HttpStatus::kInternalServerError);
    return "{\"error\": \"Internal server error\"}";
  }
}
}  // namespace

LoginUserHandler::LoginUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<StoragePgComponent>()) {}

std::string LoginUserHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return SafeHandle(request, [&]() {
    auto json = FromString(request.RequestBody());
    auto login = json["login"].As<std::string>();
    auto password = json["password"].As<std::string>();

    auto token = storage_.LoginUser(login, password);
    if (!token) {
      request.SetResponseStatus(HttpStatus::kUnauthorized);
      return std::string("{\"error\": \"Invalid credentials\"}");
    }

    ValueBuilder response;
    response["token"] = *token;
    return ToString(response.ExtractValue());
  });
}

CreateUserHandler::CreateUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<StoragePgComponent>()) {}

std::string CreateUserHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return SafeHandle(request, [&]() {
    auto json = FromString(request.RequestBody());
    User user{
        json["login"].As<std::string>(), json["first_name"].As<std::string>(),
        json["last_name"].As<std::string>(), json["password"].As<std::string>()};

    if (!storage_.AddUser(user)) {
      request.SetResponseStatus(HttpStatus::kConflict);
      return std::string("{\"error\": \"User already exists\"}");
    }
    return std::string("{\"status\": \"ok\"}");
  });
}

GetUserHandler::GetUserHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<StoragePgComponent>()) {}

std::string GetUserHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return SafeHandle(request, [&]() {
    if (!request.HasArg("login") || request.GetArg("login").empty()) {
      request.SetResponseStatus(HttpStatus::kBadRequest);
      return std::string("{\"error\": \"Missing login argument\"}");
    }
    const auto& login = request.GetArg("login");
    auto user = storage_.GetUserByLogin(login);

    if (!user) {
      request.SetResponseStatus(HttpStatus::kNotFound);
      return std::string("{\"error\": \"User not found\"}");
    }

    ValueBuilder builder;
    builder["login"] = user->login;
    builder["first_name"] = user->first_name;
    builder["last_name"] = user->last_name;
    return ToString(builder.ExtractValue());
  });
}

SearchUsersHandler::SearchUsersHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<StoragePgComponent>()) {}

std::string SearchUsersHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return SafeHandle(request, [&]() {
    if (!request.HasArg("mask") || request.GetArg("mask").empty()) {
      request.SetResponseStatus(HttpStatus::kBadRequest);
      return std::string("{\"error\": \"Missing mask argument\"}");
    }
    const auto& mask = request.GetArg("mask");
    auto users = storage_.SearchUsersByName(mask);

    ValueBuilder builder(userver::formats::json::Type::kArray);
    for (const auto& u : users) {
      ValueBuilder item;
      item["login"] = u.login;
      item["first_name"] = u.first_name;
      item["last_name"] = u.last_name;
      builder.PushBack(std::move(item));
    }
    return ToString(builder.ExtractValue());
  });
}

CreateRouteHandler::CreateRouteHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<StoragePgComponent>()) {}

std::string CreateRouteHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return SafeHandle(request, [&]() {
    auto json = FromString(request.RequestBody());
    Route route{"", json["owner_login"].As<std::string>(),
                json["points"].As<std::string>()};

    std::string id = storage_.AddRoute(std::move(route));
    ValueBuilder builder;
    builder["id"] = id;
    return ToString(builder.ExtractValue());
  });
}

GetRoutesHandler::GetRoutesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<StoragePgComponent>()) {}

std::string GetRoutesHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return SafeHandle(request, [&]() {
    if (!request.HasArg("login") || request.GetArg("login").empty()) {
      request.SetResponseStatus(HttpStatus::kBadRequest);
      return std::string("{\"error\": \"Missing login argument\"}");
    }
    const auto& login = request.GetArg("login");
    auto routes = storage_.GetRoutesByUser(login);

    ValueBuilder builder(userver::formats::json::Type::kArray);
    for (const auto& r : routes) {
      ValueBuilder item;
      item["id"] = r.id;
      item["owner_login"] = r.owner_login;
      item["points"] = r.points;
      builder.PushBack(std::move(item));
    }
    return ToString(builder.ExtractValue());
  });
}

CreateTripHandler::CreateTripHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<StoragePgComponent>()) {}

std::string CreateTripHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return SafeHandle(request, [&]() {
    auto json = FromString(request.RequestBody());
    Trip trip{"", json["route_id"].As<std::string>(), {}};

    std::string id = storage_.AddTrip(std::move(trip));
    ValueBuilder builder;
    builder["id"] = id;
    return ToString(builder.ExtractValue());
  });
}

AddUserToTripHandler::AddUserToTripHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<StoragePgComponent>()) {}

std::string AddUserToTripHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return SafeHandle(request, [&]() {
    auto json = FromString(request.RequestBody());
    auto trip_id = json["trip_id"].As<std::string>();
    auto login = json["login"].As<std::string>();

    if (!storage_.AddUserToTrip(trip_id, login)) {
      request.SetResponseStatus(HttpStatus::kNotFound);
      return std::string("{\"error\": \"Trip not found\"}");
    }
    return std::string("{\"status\": \"ok\"}");
  });
}

GetTripHandler::GetTripHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      storage_(context.FindComponent<StoragePgComponent>()) {}

std::string GetTripHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  return SafeHandle(request, [&]() {
    if (!request.HasArg("id") || request.GetArg("id").empty()) {
      request.SetResponseStatus(HttpStatus::kBadRequest);
      return std::string("{\"error\": \"Missing id argument\"}");
    }
    const auto& id = request.GetArg("id");
    auto trip = storage_.GetTrip(id);

    if (!trip) {
      request.SetResponseStatus(HttpStatus::kNotFound);
      return std::string("{\"error\": \"Trip not found\"}");
    }

    ValueBuilder builder;
    builder["id"] = trip->id;
    builder["route_id"] = trip->route_id;

    ValueBuilder participants(userver::formats::json::Type::kArray);
    for (const auto& p : trip->participants_logins) {
      participants.PushBack(p);
    }
    builder["participants"] = participants;

    return ToString(builder.ExtractValue());
  });
}

void AppendApiHandlers(userver::components::ComponentList& component_list) {
  component_list.Append<CreateUserHandler>();
  component_list.Append<GetUserHandler>();
  component_list.Append<LoginUserHandler>();
  component_list.Append<SearchUsersHandler>();
  component_list.Append<CreateRouteHandler>();
  component_list.Append<GetRoutesHandler>();
  component_list.Append<CreateTripHandler>();
  component_list.Append<AddUserToTripHandler>();
  component_list.Append<GetTripHandler>();
}

}  // namespace blablacar_service