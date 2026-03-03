package ai.seaclaw.app

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.emptyFlow
import kotlinx.coroutines.launch

/**
 * Wraps GatewayClient and manages connection state + gateway URL.
 */
class GatewayManager {

    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Main.immediate)
    private var client: GatewayClient? = null
    private val _events = MutableSharedFlow<GatewayClient.Event>()

    var gatewayUrl: String = "wss://10.0.2.2:3000/ws"

    val events: Flow<GatewayClient.Event>
        get() = _events.asSharedFlow()

    fun connect() {
        client?.disconnect()
        client = GatewayClient(gatewayUrl).also { c ->
            scope.launch {
                c.events.collect { _events.emit(it) }
            }
            c.connect()
        }
    }

    fun disconnect() {
        client?.disconnect()
        client = null
    }

    fun reconnect() {
        disconnect()
        connect()
    }

    suspend fun request(method: String, params: Map<String, Any?> = emptyMap()): Result<org.json.JSONObject> {
        val c = client ?: return Result.failure(IllegalStateException("Not connected"))
        return c.request(method, params)
    }

    fun registerPushToken(token: String) {
        client?.registerPushToken(token)
    }

    fun unregisterPushToken(token: String) {
        client?.unregisterPushToken(token)
    }
}
