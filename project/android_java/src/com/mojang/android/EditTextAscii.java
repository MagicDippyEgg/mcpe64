package com.mojang.android;
import android.content.Context;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.AttributeSet;
import android.widget.EditText;
public class EditTextAscii extends EditText implements TextWatcher {
    public EditTextAscii(Context c) { super(c); addTextChangedListener(this); }
    public EditTextAscii(Context c, AttributeSet a) { super(c,a); addTextChangedListener(this); }
    public EditTextAscii(Context c, AttributeSet a, int d) { super(c,a,d); addTextChangedListener(this); }
    @Override public void onTextChanged(CharSequence s,int st,int b,int co) {}
    public void beforeTextChanged(CharSequence s,int st,int co,int aft) {}
    public void afterTextChanged(Editable e) {
        String s=e.toString(),san=sanitize(s);
        if(!s.equals(san))e.replace(0,e.length(),san);
    }
    static public String sanitize(String s) {
        StringBuilder sb=new StringBuilder();
        for(int i=0;i<s.length();i++){char c=s.charAt(i);if(c<128)sb.append(c);}
        return sb.toString();
    }
}