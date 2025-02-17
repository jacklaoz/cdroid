#pragma once
#include <drawables/ripplebackground.h>
#include <animation/animationutils.h>
namespace cdroid{

class RippleForeground:public RippleComponent{
private:
    static constexpr int RIPPLE_ENTER_DURATION = 225;
    // Time it takes for the ripple to slide from the touch to the center point
    static constexpr int RIPPLE_ORIGIN_DURATION= 225;

    static constexpr int OPACITY_ENTER_DURATION= 75;
    static constexpr int OPACITY_EXIT_DURATION = 150;
    static constexpr int OPACITY_HOLD_DURATION = OPACITY_ENTER_DURATION + 150;

private:
    float mStartingX;
    float mStartingY;
    float mClampedStartingX;
    float mClampedStartingY;
    float mTargetX = 0;
    float mTargetY = 0;

    // Software rendering properties.
    float mOpacity = 0;

    // Values used to tween between the start and end positions.
    float mTweenRadius = 0;
    float mTweenX = 0;
    float mTweenY = 0;

    /** Whether this ripple has finished its exit animation. */
    bool mHasFinishedExit;

    /** Whether we can use hardware acceleration for the exit animation. */
    bool mUsingProperties;

    int64_t mEnterStartedAtMillis;
    bool mForceSoftware;
    float mStartRadius = 0;
    std::vector<Animator*> mRunningSwAnimators;
    Animator::AnimatorListener mAnimationListener;
private:
    float getCurrentX();
    float getCurrentY();
    void drawSoftware(Canvas& c,float origAlpha);
    void startSoftwareExit();
    void startSoftwareEnter();
    float getCurrentRadius();
    void onAnimationPropertyChanged();
    void pruneSwFinished();
protected:
    void onTargetRadiusChanged(float targetRadius)override;
    void clampStartingPosition();
public:
    RippleForeground(RippleDrawable* owner,const Rect& bounds, float startingX, float startingY, bool forceSoftware);
    ~RippleForeground();
    void getBounds(Rect& bounds);
    void move(float x, float y);
    bool hasFinishedExit()const;
    long computeFadeOutDelay();
    float getOpacity()const;
    void enter();
    void exit();
    void end();
    void draw(Canvas&,float alpha);
};

}

