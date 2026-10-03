#ifndef MODEL_LISTENER_HPP
#define MODEL_LISTENER_HPP
#include <cstdint>
class Model;
class ModelListener {
public:
    ModelListener():model(nullptr){}
    virtual ~ModelListener()=default;
    void bind(Model* m){model=m;}
    virtual void updateHR(float bpm,float trustLevel,std::uint32_t timestampMs){}
protected:
    Model* model;
};
#endif
