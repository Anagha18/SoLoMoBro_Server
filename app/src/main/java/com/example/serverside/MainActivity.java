package com.example.serverside;

import androidx.appcompat.app.AppCompatActivity;

import android.os.AsyncTask;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Collections;
import java.util.Enumeration;


public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    TextView consoleText;
    TextView netinfoText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method

        netinfoText = findViewById(R.id.netinfo);
        netinfoText.setMovementMethod(new ScrollingMovementMethod());


        consoleText = findViewById(R.id.consoleView);
        consoleText.setMovementMethod(new ScrollingMovementMethod());
        consoleText.setText("");


        Button b = findViewById(R.id.button);
        b.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new ExecCpp().execute();
            }
        });

        try {
            Enumeration<NetworkInterface> nets = NetworkInterface.getNetworkInterfaces();
            for (NetworkInterface netint : Collections.list(nets))
                displayInterfaceInformation(netint);
        }
        catch (Exception e) {
            Log.e("Neterror", e.getMessage());
        }
    }

    void displayInterfaceInformation(NetworkInterface netint) throws SocketException {

        Log.d("NET", String.format("Display name: %s\n", netint.getDisplayName()));
        netinfoText.append(String.format("Display name: %s\n", netint.getDisplayName()));

        Log.d("NET", String.format("Name: %s\n", netint.getName()));
        netinfoText.append(String.format("Name: %s\n", netint.getName()));

        Enumeration<InetAddress> inetAddresses = netint.getInetAddresses();
        for (InetAddress inetAddress : Collections.list(inetAddresses)) {
            Log.d("NET", String.format("InetAddress: %s\n", inetAddress));
            netinfoText.append(String.format("InetAddress: %s\n", inetAddress));
        }
        Log.d("NET", "\n");
        netinfoText.append("\n");
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void cppMainFunc();

    public void log_output(String output) {
//        tv.append("\n");
//        tv.append(output);
        String st = consoleText.getText().toString();

        st += output;
        consoleText.setText(st);
    }

    private class ExecCpp extends AsyncTask<Void, Integer, Void> {

        @Override
        protected Void doInBackground(Void... voids) {
            cppMainFunc();
            return null;
        }
    }

}
