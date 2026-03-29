# Домашнее задание по курсу «Архитектура программных систем»

BlablaCar-Service — это REST API приложения на С++ с использованием `userver` (in-memory хранилище данных), реализующего микросервис для управления пользователями, маршрутами и поездками. 

## Состав репозитория

- `src/` — Исходный код REST API сервиса.
- `openapi.yaml` — OpenAPI/Swagger спецификация API.
- `docker-compose.yaml` и `Dockerfile` — Скрипты для контейнеризации приложения (запуск API).
- `docker-compose.swagger.yml` — Отдельный контейнер для запуска Swagger UI.
- `tests/` — Юнит-тесты на базе `pytest`.
- `README.md` — Описание проекта.

---

## Инструкция по запуску

### Вариант 1: Запуск API локально через Docker (Рекомендуемый)

Благодаря `docker-compose`, запуск сервиса занимает одну команду:

```bash
docker-compose up --build -d
```

API сервис стартанет и будет доступен на порту `8080`.

### Вариант 2: Запуск Swagger UI для отправки запросов

Для интерактивной работы с API и просмотра документации:

```bash
docker-compose -f docker-compose.swagger.yml up -d
```

Swagger UI будет доступен по адресу: http://localhost:8081

### Вариант 3: Сборка исходников и запуск тестов локально (вне Docker)

Для локального запуска и проверки тестов:
```bash
make test-debug
make start-debug
```


---

## Примеры использования API (cURL)

Ниже приведены основные сценарии вызовов API. 

**1. Создание пользователя**
```bash
curl -X POST http://localhost:8080/v1/users \
-H "Content-Type: application/json" \
-d '{"login": "ivan", "first_name": "Ivan", "last_name": "Ivanov", "password": "pass"}'
```

**2. Авторизация (получение токена)**
```bash
curl -X POST http://localhost:8080/v1/login \
-H "Content-Type: application/json" \
-d '{"login": "ivan", "password": "pass"}'
```
*(Полученный токен передаем в заголовке `Authorization: Bearer <TOKEN>` для маршрутов и поездок).*

**3. Создание маршрута**
```bash
curl -X POST http://localhost:8080/v1/routes \
-H "Content-Type: application/json" \
-H "Authorization: Bearer <TOKEN>" \
-d '{"owner_login": "ivan", "points": "Moscow -> SPb"}'
```

**4. Поиск пользователей**
```bash
curl "http://localhost:8080/v1/users/search?mask=Ivan"
```

**5. Получение поездки по ID**
```bash
curl "http://localhost:8080/v1/trips?id=<ID_ПОЕЗДКИ>"
```
