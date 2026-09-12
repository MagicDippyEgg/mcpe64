package com.mojang.minecraftpe;
import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
public class Minecraft_Market extends Activity {
    @Override protected void onCreate(Bundle s){super.onCreate(s);startActivity(new Intent(this, MainActivity.class));finish();}
}