FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc)

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/mcp_sentinel /app/mcp_sentinel

RUN mkdir -p /app/logs

ENV MCP_SENTINEL_PORT=8080
ENV MCP_SENTINEL_AUDIT_LOG_PATH=/app/logs/audit.log

EXPOSE 8080

CMD ["/app/mcp_sentinel"]