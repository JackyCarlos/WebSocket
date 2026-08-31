# WebSocket

websocket server implementation compliant to [RFC6455](https://www.rfc-editor.org/rfc/rfc6455.html)

## Autobahn testing

```
docker run -it --rm --network host -v "$(pwd)/config:/config" -v "$(pwd)/reports:/reports" crossbario/autobahn-testsuite:latest wstest -m fuzzingclient -s /config/fuzzingclient.json
```
