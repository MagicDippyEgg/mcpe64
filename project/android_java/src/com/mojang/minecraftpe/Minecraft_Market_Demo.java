package com.mojang.minecraftpe;
import android.content.Intent;
import android.net.Uri;
public class Minecraft_Market_Demo extends MainActivity {
    @Override public void buyGame(){startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id=com.mojang.minecraftpe")));}
    @Override protected boolean isDemo(){return true;}
}