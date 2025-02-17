#include <animation/animatorset.h>
#include <animation/animatorinflater.h>
#include <core/typedvalue.h>
#include <expat.h>
#include <cdlog.h>

namespace cdroid{

Animator* AnimatorInflater::loadAnimator(Context* context,const std::string&resid){
    return createAnimatorFromXml(context,resid);
}

StateListAnimator* AnimatorInflater::loadStateListAnimator(Context* context,const std::string&resid){
    return createStateListAnimatorFromXml(context,resid);
}

typedef struct{
    std::string name;
    Animator*animator;
    std::vector<int>state;
    AttributeSet atts;
}AnimNode;
typedef struct{
    Context*context;
    std::vector<AnimNode>items;
    std::string package;
    Animator*animator;
    std::vector<Animator*>mChildren;
    StateListAnimator*statelistAnimator;
    AnimNode*fromTop(int pos){
        int size =(int)items.size();
        if(size+pos<0||pos>0)return nullptr;
        return &items.at(size+pos);
    }
}AnimatorParseData;

class AnimatorInflater::AnimatorParser{
public:
    static void startElement(void *userData, const XML_Char *name, const XML_Char **satts){
        AnimatorParseData*pd =(AnimatorParseData*)userData;
        AnimNode an;
        AttributeSet atts;
        an.name = name;
        an.animator = nullptr;
        an.atts.setContext(pd->context,pd->package);
        an.atts.set(satts);
        if(strcmp(name,"selector")==0){
            pd->statelistAnimator = new StateListAnimator();
        }if(strcmp(name,"item")==0){
            StateSet::parseState(an.state,an.atts);
        }else if(strcmp(name,"objectAnimator")==0){
            an.animator = AnimatorInflater::loadObjectAnimator(pd->context,an.atts);
        }else if(strcmp(name,"animator")==0){
            an.animator = AnimatorInflater::loadValueAnimator(pd->context,an.atts,nullptr);
        }else if(strcmp(name,"set")==0){
            an.animator = new AnimatorSet();
            pd->mChildren.clear();
        }else{
            an.animator = AnimatorInflater::loadObjectAnimator(pd->context,an.atts);
        }

        if( (pd->items.size()==0) && (pd->statelistAnimator==nullptr) )
            pd->animator = an.animator;

        LOGD("%s",(std::string(pd->items.size()*4,' ')+name).c_str());
        pd->items.push_back(an);
    }

    static void __endElement(void *userData, const XML_Char *name){
        AnimatorParseData*pd =(AnimatorParseData*)userData;
        AnimNode back = pd->items.back();
        if(strcmp(name,"item")==0){
            pd->statelistAnimator->addState(back.state,back.animator);
        }else if((strcmp(name,"objectAnimator")==0)||(strcmp(name,"animator")==0)){
            AnimNode*parent = pd->fromTop(-2);
            if(parent->name.compare("set")==0){
                pd->mChildren.push_back(back.animator);
            }
        }else if((strcmp(name,"set")==0)&&pd->items.size()){
            AnimNode* parent = pd->fromTop(pd->statelistAnimator?-2:-1);
            if(parent->name.compare("item")){
                const int together = back.atts.getInt("ordering",std::map<const std::string,int>{
                    {"together",0}, {"sequentially",1}},0);
                AnimatorSet*aset = (AnimatorSet*)parent->animator;
                if(together==0) aset->playTogether(pd->mChildren);
                else aset->playSequentially(pd->mChildren);
            }else{
                parent->animator=back.animator;
            }
        }else{
            AnimNode* parent = pd->fromTop(pd->statelistAnimator?-2:-1);
            if(parent&&(parent->name.compare("set")==0)){
                pd->mChildren.push_back(back.animator);
            }
        }
        pd->items.pop_back();
        LOGD("%s",(std::string(pd->items.size()*4,' ')+name).c_str());
    }
};

Animator* AnimatorInflater::createAnimatorFromXml(Context*ctx,const std::string&resid){
    int len;
    char buf[128];
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');
    AnimatorParseData pd;
    pd.context = ctx;
    pd.animator  = nullptr;
    pd.statelistAnimator = nullptr;
    std::unique_ptr<std::istream>stream = ctx->getInputStream(resid,&pd.package);
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, AnimatorParser::startElement, AnimatorParser::__endElement);
    do {
        stream->read(buf,sizeof(buf));
        len=stream->gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return nullptr;
        }
    } while(len!=0);
    XML_ParserFree(parser);
    return pd.animator;
}

StateListAnimator* AnimatorInflater::createStateListAnimatorFromXml(Context*ctx,const std::string&resid){
    int len;
    char buf[128];
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');
    AnimatorParseData pd;
    pd.context = ctx;
    pd.animator  = nullptr;
    pd.statelistAnimator = nullptr;
    std::unique_ptr<std::istream>stream = ctx->getInputStream(resid,&pd.package);
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, AnimatorParser::startElement, AnimatorParser::__endElement);
    do {
        stream->read(buf,sizeof(buf));
        len = stream->gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es = XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return nullptr;
        }
    } while(len!=0);
    XML_ParserFree(parser);
    return pd.statelistAnimator;
}

bool AnimatorInflater::isColorType(int type) {
    return (type >= TypedValue::TYPE_FIRST_COLOR_INT) && (type <= TypedValue::TYPE_LAST_COLOR_INT);
}

int AnimatorInflater::valueTypeFromPropertyName(const std::string& name){
    static const std::map<const std::string,int>valueTypes = {
       {"x",(int)TypedValue::TYPE_DIMENSION},
       {"y",(int)TypedValue::TYPE_DIMENSION},
       {"z",(int)TypedValue::TYPE_DIMENSION},
       {"scale" ,(int)TypedValue::TYPE_FLOAT},
       {"scaleX",(int)TypedValue::TYPE_FLOAT},
       {"scaleY",(int)TypedValue::TYPE_FLOAT},
       {"scaleZ",(int)TypedValue::TYPE_FLOAT},
       {"rotation",(int)TypedValue::TYPE_FLOAT},
       {"rotationX",(int)TypedValue::TYPE_FLOAT},
       {"rotationY",(int)TypedValue::TYPE_FLOAT},
       {"rotationZ",(int)TypedValue::TYPE_FLOAT},
       {"translationX",(int)TypedValue::TYPE_DIMENSION},
       {"translationY",(int)TypedValue::TYPE_DIMENSION},
       {"translationX",(int)TypedValue::TYPE_DIMENSION}
    };
    auto it = valueTypes.find(name);
    if(it!=valueTypes.end())return it->second;
    return TypedValue::TYPE_NULL;
}

PropertyValuesHolder*AnimatorInflater::getPVH(const AttributeSet&atts, int valueType,const std::string& propertyName){
    PropertyValuesHolder* returnValue = nullptr;
    const std::string sFrom = atts.getString("valueFrom");
    const std::string sTo = atts.getString("valueTo");
    const bool hasFrom = !sFrom.empty();
    const bool hasTo   = !sTo.empty();
    const int fromType = valueTypeFromPropertyName(propertyName);
    const int toType = fromType;
    const bool getFloats = (valueType==VALUE_TYPE_FLOAT);

    if (valueType == VALUE_TYPE_PATH) {
        std::string fromString = atts.getString("valueFrom");
        std::string toString = atts.getString("valueTo");
        /*PathParser.PathData nodesFrom = fromString == null ? null : new PathParser.PathData(fromString);
        PathParser.PathData nodesTo = toString == null  ? null : new PathParser.PathData(toString);

        if (nodesFrom != null || nodesTo != null) {
            if (nodesFrom != null) {
                TypeEvaluator evaluator = new PathDataEvaluator();
                if (nodesTo != null) {
                    if (!PathParser.canMorph(nodesFrom, nodesTo)) {
                        throw std::runtime_error(std::string(" Can't morph from") + fromString + " to " + toString);
                    }
                    returnValue = PropertyValuesHolder::ofObject(propertyName, evaluator, nodesFrom, nodesTo);
                } else {
                    returnValue = PropertyValuesHolder::ofObject(propertyName, evaluator, (Object) nodesFrom);
                }
            } else if (nodesTo != null) {
                TypeEvaluator evaluator = new PathDataEvaluator();
                returnValue = PropertyValuesHolder::ofObject(propertyName, evaluator, (Object) nodesTo);
            }
        }*/
    } else {
        /*TypeEvaluator evaluator = nullptr;
        // Integer and float value types are handled here.
        if (valueType == VALUE_TYPE_COLOR) {
            // special case for colors: ignore valueType and get ints
            evaluator = ArgbEvaluator.getInstance();
        }*/
        if (getFloats) {
            float valueFrom,valueTo;
            if (hasFrom) {
                if(fromType==TypedValue::TYPE_DIMENSION){
                    valueFrom = atts.getDimension("valueFrom", 0);
                }else{
                    valueFrom = atts.getFloat("valueFrom",0);
                }
                if (hasTo) {
                    if(toType==TypedValue::TYPE_DIMENSION)
                        valueTo = atts.getDimension("valueTo", 0);
                    else
                        valueTo = atts.getFloat("valueTo",0);
                    returnValue = PropertyValuesHolder::ofFloat(propertyName,{valueFrom, valueTo});
                } else {
                    returnValue = PropertyValuesHolder::ofFloat(propertyName,{valueFrom});
                }
            } else {
                if(toType==TypedValue::TYPE_DIMENSION)
                    valueTo = atts.getDimension("valueTo", 0);
                else
                    valueTo = atts.getDimension("valueTo",0);
                returnValue = PropertyValuesHolder::ofFloat(propertyName, {valueTo});
            }
        } else {
            int valueFrom,valueTo;
            if (hasFrom) {
                if (fromType == TypedValue::TYPE_DIMENSION) {
                    valueFrom = (int) atts.getDimension("valueFrom", 0);
                } else if (isColorType(fromType)) {
                    valueFrom = atts.getColor("valueFrom", 0);
                } else {
                    valueFrom = atts.getInt("valueFrom", 0);
                }
                if (hasTo) {
                    if (toType == TypedValue::TYPE_DIMENSION) {
                        valueTo = (int) atts.getDimension("valueTo", 0);
                    } else if (isColorType(toType)) {
                        valueTo = atts.getColor("valueTo", 0);
                    } else {
                        valueTo = atts.getInt("valueTo", 0);
                    }
                    returnValue = PropertyValuesHolder::ofInt(propertyName, {valueFrom, valueTo});
                } else {
                    returnValue = PropertyValuesHolder::ofInt(propertyName, {valueFrom});
                }
            } else {
                if (hasTo) {
                    if (toType == TypedValue::TYPE_DIMENSION) {
                        valueTo = (int) atts.getDimension("valueTo", 0);
                    } else if (isColorType(toType)) {
                        valueTo = atts.getColor("valueTo", 0);
                    } else {
                        valueTo = atts.getInt("valueTo", 0);
                    }
                    returnValue = PropertyValuesHolder::ofInt(propertyName, {valueTo});
                }
            }
        }
        /*if (returnValue != null && evaluator != null) {
            returnValue.setEvaluator(evaluator);
        }*/
    }
    return returnValue;
}

ObjectAnimator* AnimatorInflater::loadObjectAnimator(Context*ctx,const AttributeSet& atts){
    ObjectAnimator*anim = new ObjectAnimator();
    loadValueAnimator(ctx,atts,anim);
    return anim;
}

ValueAnimator*  AnimatorInflater::loadValueAnimator(Context*context,const AttributeSet& atts, ValueAnimator*anim){
    const std::string propertyName = atts.getString("propertyName");
    if(anim==nullptr){
        LOGD("propertyName=%s",propertyName.c_str());
        anim = new ValueAnimator();
    }
    const int valueType = atts.getInt("valueType",std::map<const std::string,int>{
            {"floatType",(int)VALUE_TYPE_FLOAT},  {"intType",(int)VALUE_TYPE_INT},
            {"colorType",(int)VALUE_TYPE_COLOR},  {"pathType",(int)VALUE_TYPE_PATH}
        },(int)VALUE_TYPE_UNDEFINED);

    anim->setDuration(atts.getInt("duration",300));
    anim->setStartDelay(atts.getInt("startOffset",0));
    anim->setRepeatCount(atts.getInt("repeatCount",0));
    anim->setRepeatMode(atts.getInt("repeatMode",std::map<const std::string,int>{
        {"restart" , (int)ValueAnimator::RESTART},
        {"reverse" , (int)ValueAnimator::REVERSE},
        {"infinite", (int)ValueAnimator::INFINITE}
    },ValueAnimator::RESTART));

    PropertyValuesHolder*pvh = getPVH(atts,valueType,propertyName);
    if(pvh)
        anim->setValues({pvh});
    return anim;
}

}
