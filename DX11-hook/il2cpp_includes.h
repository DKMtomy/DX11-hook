#ifndef IL2CPP_INCLUDES_H
#define IL2CPP_INCLUDES_H

#pragma once

// Forward declarations for IL2CPP namespaces and classes
namespace IL2CPP {
    namespace UnityAPI {}
    class SystemTypeCache;
    namespace Utils {
        class Hash;
        class VTable;
        class Helper;
    }
    namespace API {
        class Domain;
        class Class;
        class ResolveCall;
        class String;
        class Thread;
        class Callback;
    }
}

// Forward declarations for Unity structures and classes
namespace Unity {
    struct il2cppArray;
    struct il2cppDictionary;
    class CCamera;
    class CComponent;
    class CGameObject;
    class CLayerMask;
    class CObject;
    class CRigidbody;
    class CTransform;

    namespace API {
        class Object;
        class GameObject;
        class Camera;
        class Component;
        class LayerMask;
        class Rigidbody;
        class Transform;
    }
}

#endif // IL2CPP_INCLUDES_H