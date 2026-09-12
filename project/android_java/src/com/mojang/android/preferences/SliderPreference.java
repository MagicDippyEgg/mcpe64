package com.mojang.android.preferences;
import android.content.Context;
import android.content.res.Resources;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;
public class SliderPreference extends DialogPreference implements SeekBar.OnSeekBarChangeListener {
    private static final String NS="http://schemas.android.com/apk/res/android";
    private Context _ctx; private TextView _tv; private SeekBar _sb;
    private String _suf; private int _def,_max,_val,_min;
    public SliderPreference(Context ctx, AttributeSet a){
        super(ctx,a); _ctx=ctx;
        _suf=gStr(a,NS,"text",""); _def=gInt(a,NS,"defaultValue",0);
        _max=gInt(a,NS,"max",100); _min=gInt(a,null,"min",0);
        setDefaultValue(_def);
    }
    @Override protected View onCreateDialogView(){
        LinearLayout l=new LinearLayout(_ctx); l.setOrientation(LinearLayout.VERTICAL); l.setPadding(6,6,6,6);
        _tv=new TextView(_ctx); _tv.setGravity(Gravity.CENTER_HORIZONTAL); _tv.setTextSize(32);
        l.addView(_tv,new LinearLayout.LayoutParams(-1,-2));
        _sb=new SeekBar(_ctx); _sb.setOnSeekBarChangeListener(this);
        l.addView(_sb,new LinearLayout.LayoutParams(-1,-2));
        if(shouldPersist())_val=getPersistedInt(_def);
        _sb.setMax(_max); _sb.setProgress(_val); return l;
    }
    @Override protected void onSetInitialValue(boolean r,Object d){
        super.onSetInitialValue(r,d);
        _val=r?(shouldPersist()?getPersistedInt(_def):0):(Integer)d;
    }
    public void onProgressChanged(SeekBar s,int v,boolean f){
        _val=v+_min; _tv.setText(_val+_suf);
        if(shouldPersist())persistInt(_val); callChangeListener(Integer.valueOf(_val));
    }
    public void onStartTrackingTouch(SeekBar s) {}
    public void onStopTrackingTouch(SeekBar s) {}
    private int gInt(AttributeSet a,String ns,String n,int d){int id=a.getAttributeResourceValue(ns,n,0);return id!=0?getContext().getResources().getInteger(id):a.getAttributeIntValue(ns,n,d);}
    private String gStr(AttributeSet a,String ns,String n,String d){int id=a.getAttributeResourceValue(ns,n,0);if(id!=0)return getContext().getResources().getString(id);String v=a.getAttributeValue(ns,n);return v!=null?v:d;}
}