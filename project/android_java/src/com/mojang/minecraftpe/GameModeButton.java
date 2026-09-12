package com.mojang.minecraftpe;
import com.mojang.android.StringValue;
import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.TextView;
import android.widget.ToggleButton;
public class GameModeButton extends ToggleButton implements OnClickListener,StringValue {
    static final int Creative=0,Survival=1;
    private int _type=0; private boolean _attached=false;
    public GameModeButton(Context c,AttributeSet a){super(c,a);setOnClickListener(this);}
    public void onClick(View v){_update();}
    @Override protected void onFinishInflate(){super.onFinishInflate();_update();}
    @Override protected void onAttachedToWindow(){if(!_attached){_update();_attached=true;}}
    private void _update(){_set(isChecked()?Survival:Creative);}
    private void _set(int i){
        _type=i<Creative?Creative:(i>Survival?Survival:i);
        int id=_type==Survival?R.string.gamemode_survival_summary:R.string.gamemode_creative_summary;
        String desc=getContext().getString(id);
        View v=getRootView().findViewById(R.id.labelGameModeDesc);
        if(desc!=null&&v instanceof TextView)((TextView)v).setText(desc);
    }
    public String getStringValue(){return new String[]{"creative","survival"}[_type];}
    static public String getStringForType(int i){int c=i<Creative?Creative:(i>Survival?Survival:i);return new String[]{"creative","survival"}[c];}
}