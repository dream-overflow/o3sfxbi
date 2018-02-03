/**
 * @brief FBX importer adapter to O3S implementation
 * @copyright Copyright (C) 2018 Dream Overflow. All rights reserved.
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2018-01-20
 * @details
 */

#include "o3sadapter.h"
#include "proxy/headerproxy.h"
#include "proxy/definitionsproxy.h"
#include "proxy/objectsproxy.h"
#include "proxy/modelproxy.h"
#include "proxy/relationsproxy.h"
#include "proxy/globalsettingsproxy.h"
#include "proxy/connectionsproxy.h"
// #include "proxy/takesproxy.h"
// #include "proxy/takeproxy.h"

#include "proxy/modelproxy.h"
#include "proxy/materialproxy.h"
#include "proxy/cameraproxy.h"
#include "proxy/lightproxy.h"
#include "proxy/textureproxy.h"
#include "proxy/nodeattributeproxy.h"
#include "proxy/geometryproxy.h"

#include <o3d/studio/common/objectref.h>
#include <o3d/studio/common/application.h>

#include <o3d/studio/common/component/componentregistry.h>
#include <o3d/studio/common/component/spacialnodehub.h>
#include <o3d/studio/common/component/meshhub.h>
#include <o3d/studio/common/component/camerahub.h>
// #include <o3d/studio/common/component/lighthub.h>
// #include <o3d/studio/common/component/materialhub.h>
// #include <o3d/studio/common/component/texturehub.h>
// #include <o3d/studio/common/component/skeletonhub.h>
// #include <o3d/studio/common/component/animationlayerhub.h>
// #include <o3d/studio/common/component/animationtrackhub.h>
// #include <o3d/studio/common/component/animationnodehub.h>

#include <o3d/studio/common/workspace/project.h>
#include <o3d/studio/common/objectref.h>
#include <o3d/studio/common/component/dummyhub.h>

using namespace o3d::studio::fbxi;

O3SAdapter::O3SAdapter(Parser *parser,
        o3d::studio::common::ImporterOption *options,
        o3d::studio::common::Entity *parent,
        FbxImportDefinition* def) :
    m_parser(parser),
    m_options(options),
    m_parent(parent),
    m_def(def),
    m_upAxis(0, 1, 0),
    m_frontAxis(0, 0, 1),
    m_unitScale(1.0)
{
    O3D_ASSERT(parser);
    O3D_ASSERT(options);
    O3D_ASSERT(parent);
    O3D_ASSERT(def);
}

O3SAdapter::~O3SAdapter()
{
    for (auto it = m_hubs.begin(); it != m_hubs.end(); ++it) {
        delete it->second;
    }
}

o3d::Bool O3SAdapter::processImport()
{
    common::Hub *topLevelHub = nullptr;

    // read interesting definitions
    setupDef();

    if (m_parent->ref().light().baseTypeOf(common::TypeRef::project())) {
        common::Project *project = static_cast<common::Project*>(m_parent);
    } else if (m_parent->ref().light().baseTypeOf(common::TypeRef::hub())) {
        topLevelHub = static_cast<common::Hub*>(m_parent);
    }

    setupHub(topLevelHub);

    return True;
}

o3d::Bool O3SAdapter::processImportLazy()
{
    // @todo
    return False;
}

o3d::studio::common::Hub *O3SAdapter::hub(o3d::Int64 uid)
{
    auto it = m_hubs.find(uid);
    if (it != m_hubs.end()) {
        return it->second;
    }

    return nullptr;
}

void O3SAdapter::setupDef()
{
    common::Project *project = m_parent->project();

    // global informations about this document
    FBXNode *node = m_parser->child("FBXHeaderExtension");
    if (node) {
        HeaderProxy *hp = new HeaderProxy(node);
        m_def->m_creator = hp->creator();
        m_def->m_creationTimestamp = hp->creationTimeStamp();

        delete hp;
    }

    // global settings of the data format (for transform, and details)
    node = m_parser->child("GlobalSettings");
    if (node) {
        GlobalSettingsProxy *gs = new GlobalSettingsProxy(node);
        m_unitScale = gs->unitScale();
        m_upAxis = gs->upAxis();
        m_frontAxis = gs->frontAxis();
        m_startTime = gs->timeSpanStart();
        m_endTime = gs->timeSpanEnd();

        m_def->m_ambientColor = gs->ambientColor();

        delete gs;
    }

    // definitions of the templates
    node = m_parser->child("Definitions");
    if (node) {
        DefinitionsProxy *definitions = new DefinitionsProxy(node);

        // @todo TemplateProxy ... apply template to objects after ...
        Int32 count = definitions->count();

        delete definitions;
    }

    node = m_parser->child("Objects");
    if (node) {
        ObjectsProxy *objects = new ObjectsProxy(node);

        for (UInt32 i = 0; i < objects->numObjects(); ++i) {
            switch (objects->objectType(i)) {
                case ObjectsProxy::OBJECT_CAMERA:
                {
                    CameraProxy *cp = objects->camera(i);
//                    common::Component *component = common::Application::instance()->components().component("o3s::common::component::camerahub");
//                    common::CameraHub *hub = static_cast<common::CameraHub*>(component->buildHub(cp->name(), project, project));

                    // @todo ortho, fov...

            //        m_hubs[cp->uuid()] = hub;
                    delete cp;
                }
                    break;
                case ObjectsProxy::OBJECT_GEOMETRY:
                {
                    GeometryProxy *gp = objects->geometry(i);
               //     common::MeshHub *hub = new common::MeshHub(gp->name());

                    // @todo explode vertices, uvs...
                    // hub->setVertices(gp->vertices());

                    // probably used by a model node
                //    m_hubs[gp->uuid()] = hub;
                    delete gp;
                }
                    break;
                case ObjectsProxy::OBJECT_LIGHT:
                {
                    LightProxy *lp = objects->light(i);
                    // common::LightHub *hub = new common::LightHub(mp->name());

                    // @todo ambient, diffuse, specular, type, size...

                    // m_hubs[mp->typeName()] = hub;
                    delete lp;
                }
                    break;
                case ObjectsProxy::OBJECT_MATERIAL:
                {
                    MaterialProxy *mp = objects->material(i);
//                    common::MaterialHub *hub = new common::MaterialHub(mp->name());

//                    // @todo ambient, diffuse, specular, transparency...

//                    m_hubs[mp->uuid()] = hub;

                    delete mp;
                }
                    break;
                case ObjectsProxy::OBJECT_MODEL:
                {
                    ModelProxy *mp = objects->model(i);

                    common::Component *component = common::Application::instance()->components().component("o3s::common::component::spacialhub");
                    common::SpacialNodeHub *hub = static_cast<common::SpacialNodeHub*>(component->buildHub("Node " + mp->name(), project, project));

                    hub->setPosition(0, mp->position());
                    hub->setRotation(0, mp->rotation());
                    hub->setScale(0, mp->scale());

                    // parent will be know during connections
                    m_hubs[mp->uuid()] = hub;

                    delete mp;
                }
                    break;
                case ObjectsProxy::OBJECT_NODE_ATTRIBUTE:
                {
                    NodeAttributeProxy *np = objects->nodeAttribute(i);

                    common::Component *component = common::Application::instance()->components().component("o3s::common::component::spacialhub");
                    common::SpacialNodeHub *hub = static_cast<common::SpacialNodeHub*>(component->buildHub(np->name(), project, project));

                    // parent will be know during connections
                    m_hubs[np->uuid()] = hub;

                    delete np;
                }
                    break;
                case ObjectsProxy::OBJECT_TEXTURE:
                {
                    TextureProxy *tp = objects->texture(i);
                    // @todo add a TextureHub ?

                    // @todo

                    delete tp;
                }
                    break;
                default:
                    break;
            }
        }

        delete objects;
    }

    m_def->m_unit = m_unitScale;
}

//void O3SAdapter::setupAsset(common::Asset* asset)
//{
//    FBXNode *node = m_parser->child("FBXHeaderExtension");
//    if (node) {
//        HeaderProxy *hp = new HeaderProxy(node);

//        asset->setCreator(hp->creator());
//        asset->setCreationTimeStamp(hp->creationTimeStamp());

//        delete hp;
//    }
//}

void O3SAdapter::setupHub(common::Hub* rootHub)
{
    common::Project *project = m_parent->project();

    FBXNode *node = m_parser->child("Connections");
    if (node) {
        ConnectionsProxy *connections = new ConnectionsProxy(node);

        // setup connections between objects
        for (UInt32 i = 0; i < connections->numConnections(); ++i) {
            if (connections->connectionType(i) == ConnectionsProxy::CONN_OO) {
                Int64 parentUid, childUid;
                connections->objectRelation(i, parentUid, childUid);

                common::Hub* parentHub = hub(parentUid);
                common::Hub* childHub = hub(childUid);

                // not imported children ignore the connection
                if (!childHub) {
                    continue;
                }

                if (!childHub->ref().light().isValid()) {
                    childHub->setRef(common::ObjectRef::buildRef(m_parent->project(), childHub->typeRef()));
                }

                // if parent is null, means root
                if (!parentHub) {
                    if (rootHub) {
                        // reparent
                        childHub->setParent(rootHub);
                        rootHub->addHub(childHub);
                    } else {
                        // reparent
                        childHub->setParent(project);
                        project->addHub(childHub);
                    }
                } else {
                    // reparent
                    childHub->setParent(parentHub);
                    parentHub->addHub(childHub);
                }
            } else if (connections->connectionType(i) == ConnectionsProxy::CONN_OP) {
                // @todo mostly texture
            }
        }

        delete connections;
    }

    // clear to avoid theirs destruction at destructor
    m_hubs.clear();
}
