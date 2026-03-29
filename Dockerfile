FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg-dev:latest

WORKDIR /app
COPY . .

# Build the project in release mode
RUN cmake --preset release && \
    cmake --build build-release -j $(nproc)

EXPOSE 8080

CMD ["/app/build-release/blablacar_service", "--config", "/app/configs/static_config.yaml"]
