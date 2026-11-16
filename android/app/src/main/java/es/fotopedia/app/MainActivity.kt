package es.fotopedia.app

import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

// Etiqueta de los mensajes de la app. Se ven con: adb logcat -s Fotopedia
private const val ETIQUETA = "Fotopedia"

class MainActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // La clave no se escribe nunca en el log: solo si esta o no.
        Log.i(ETIQUETA, "App arrancada. IP de la placa por defecto: ${BuildConfig.ARDUINO_IP}")
        Log.i(ETIQUETA, "Clave de Vision cargada: ${BuildConfig.VISION_API_KEY.isNotEmpty()}")

        setContent {
            MaterialTheme {
                PantallaInicio(ipArduino = BuildConfig.ARDUINO_IP)
            }
        }
    }
}

// Pantalla de arranque: solo sirve para comprobar que la app compila, se instala y abre.
@Composable
fun PantallaInicio(ipArduino: String) {
    Surface(modifier = Modifier.fillMaxSize()) {
        Column(
            modifier = Modifier.fillMaxSize().padding(24.dp),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.CenterHorizontally,
        ) {
            Text(text = "Fotopedia 3000", style = MaterialTheme.typography.headlineLarge)
            Text(text = "Placa: $ipArduino", style = MaterialTheme.typography.bodyLarge)
        }
    }
}
