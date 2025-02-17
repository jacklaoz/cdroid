#include <cdroid.h>
#include <cdlog.h>
#include <fstream>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    cdroid::Context*ctx=&app;
    Window*w=new Window(0,0,-1,-1);
    w->setId(1);
    Drawable*d=nullptr;
    StateListDrawable*sld;
    CompoundButton*chk;
    LOGD("test LOGF %d",__LINE__);
    LOG(DEBUG)<<"Test Stream(DEBUG)";
#if 10
    Button *btn=new Button("Button",120,60);
    d=ctx->getDrawable("cdroid:drawable/btn_default.xml");
    sld=dynamic_cast<StateListDrawable*>(d);
    w->setBackgroundColor(0xFF101112);
    btn->setOnTouchListener([&argc](View&v,MotionEvent&e){
        const bool down=e.getAction()==MotionEvent::ACTION_DOWN;
        AnimatorSet*aset= new AnimatorSet();
        Animator* alpha = ObjectAnimator::ofFloat(&v, "alpha", {0.f});
        Animator* scale = ObjectAnimator::ofFloat(&v, "scaleX", {1.5f});
        alpha->setDuration(2000);
        scale->setDuration(2000);
        if(argc%2)aset->playTogether({alpha,scale});
        else aset->playSequentially({scale,alpha});
        aset->start();
        return false;
    });
    
    LOGD("%p statecount=%d",sld,sld->getStateCount());
    btn->setBackground(d);
    btn->setBackgroundTintList(ctx->getColorStateList("cdroid:color/textview"));
    btn->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);
    btn->setOnClickListener([](View&v){LOGD(" Button Clicked ");});
    btn->setOnLongClickListener([](View&v)->bool{LOGD(" Button LongClicked ");return true;});
    w->addView(btn).setId(100).setPos(50,60);

    ShapeDrawable*sd=new ShapeDrawable();
    sd->setShape(new ArcShape(0,360));
    sd->getShape()->setGradientColors({0x20FFFFFF,0xFFFFFFFF,0x00FFFFFF});//setSolidColor(0x800000FF);
    RippleDrawable*rp=new RippleDrawable(ColorStateList::valueOf(0x80222222),new ColorDrawable(0x8000FF00),sd);
    btn=new Button("RippleButton",300,64);
    btn->setMinimumHeight(64);
    btn->setBackground(rp);
    btn->setClickable(true);
    w->addView(btn).setId(101).setPos(200,60);

    btn=new ToggleButton(120,40);
    d=ctx->getDrawable("cdroid:drawable/btn_toggle_bg.xml");
    btn->setBackground(d);
    btn->setTextColor(ctx->getColorStateList("cdroid:color/textview"));
    ((ToggleButton*)btn)->setTextOn("ON");
    ((ToggleButton*)btn)->setTextOff("Off");
    w->addView(btn).setId(101).setPos(200,150).setClickable(true);

    chk=new CheckBox("CheckME",200,60);
    d=ctx->getDrawable("cdroid:drawable/btn_check.xml");
    chk->setButtonDrawable(d);
    chk->setChecked(true);
    w->addView(chk).setPos(350,150);
	
    /*AnalogClock*clk=new AnalogClock(300,300);
    d=ctx->getDrawable("cdroid:drawable/analog.xml");
    clk->setClockDrawable(d,AnalogClock::DIAL);
    d=ctx->getDrawable("cdroid:drawable/analog_second.xml");
    clk->setClockDrawable(d,AnalogClock::SECOND);
    w->addView(clk).setPos(600,300);*/

#if 1 
    chk=new RadioButton("Radio",120,60);
    Drawable*dr=ctx->getDrawable("cdroid:drawable/btn_radio.xml");
    chk->setButtonDrawable(dr);
    chk->setChecked(true);
    w->addView(chk).setPos(600,150);
	
    EditText*edt=new EditText("Edit Me!",200,60);
    d=ctx->getDrawable("cdroid:drawable/edit_text.xml");//editbox_background.xml");
    edt->setBackground(d);
    edt->setTextColor(ctx->getColorStateList("cdroid:color/textview.xml"));
    w->addView(edt).setId(102).setPos(800,60).setKeyboardNavigationCluster(true);
#endif
///////////////////////////////////////////////////////////
#if 1
    ProgressBar*pb=new ProgressBar(500,40);
    d=ctx->getDrawable("cdroid:drawable/progress_horizontal.xml");
    LOGD("progress_horizontal drawable=%p",d);
    pb->setProgressDrawable(d);
    pb->setProgress(34);
    pb->setSecondaryProgress(15);
    w->addView(pb).setPos(50,150);

#endif
#if 1
    //////////////////////////////////////////////////////////    
    ProgressBar*pb2=new ProgressBar(72,72);
    d=ctx->getDrawable("cdroid:drawable/progress_large.xml");
    pb2->setIndeterminateDrawable(d);
    LOGD("Indeterminate drawable=%p",d);
    w->addView(pb2).setId(104).setPos(50,450);
    pb2->setProgressDrawable(new ColorDrawable(0xFF112233));
    pb2->setIndeterminate(true);
#endif
#endif
#if 1
    SeekBar*sb=new SeekBar(800,30);
    SeekBar*sb2=new SeekBar(800,60);

    d=ctx->getDrawable("cdroid:drawable/progress_horizontal.xml");
    sb->setProgressDrawable(d);
    sb2->setProgressDrawable(d->getConstantState()->newDrawable());

    d=ctx->getDrawable("cdroid:drawable/seek_thumb.xml");
    sb->setThumb(d);
    sb2->setThumb(d->getConstantState()->newDrawable());
    d=ctx->getDrawable("cdroid:drawable/seekbar_tick_mark.xml");
    sb->setTickMark(d);
    sb2->setTickMark(d->getConstantState()->newDrawable());
    w->addView(sb).setId(200).setPos(150,250).setKeyboardNavigationCluster(true);
    w->addView(sb2).setId(201).setPos(150,300);

#endif
    return app.exec();
}
