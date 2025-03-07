/*
 * esmini - Environment Simulator Minimalistic
 * https://github.com/esmini/esmini
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) partners of Simulation Scenarios
 * https://sites.google.com/view/simulationscenarios
 */

#pragma once

#include <string>
#include "CommonMini.hpp"
#include "OSCProperties.hpp"
#include "Parameters.hpp"

#define CONTROLLER_BASE_TYPE_NAME "ControllerClass"
#define CONTROLLER_BASE_TYPE_ID   -1

namespace scenarioengine
{
    // Forward declarations
    class ScenarioPlayer;
    class ScenarioGateway;
    class ScenarioEngine;
    class Entities;
    class Object;

    // base class for controllers
    class Controller
    {
    public:
        enum Type
        {
            CONTROLLER_TYPE_DEFAULT,
            CONTROLLER_TYPE_EXTERNAL,
            CONTROLLER_TYPE_FOLLOW_GHOST,
            CONTROLLER_TYPE_FOLLOW_ROUTE,
            CONTROLLER_TYPE_INTERACTIVE,
            CONTROLLER_TYPE_SLOPPY_DRIVER,
            CONTROLLER_TYPE_SUMO,
            CONTROLLER_TYPE_REL2ABS,
            CONTROLLER_TYPE_ACC,
            CONTROLLER_TYPE_ALKS,
            CONTROLLER_TYPE_UDP_DRIVER,
            CONTROLLER_TYPE_ECE_ALKS_REF_DRIVER,
            CONTROLLER_ALKS_R157SM,
            CONTROLLER_TYPE_LOOMING,
            N_CONTROLLER_TYPES,
            GHOST_RESERVED_TYPE       = 100,
            USER_CONTROLLER_TYPE_BASE = 1000,
        };

        typedef enum
        {
            MODE_NONE,      // Controller not available or it is not active
            MODE_OVERRIDE,  // Actions from the scenario are not applied, default
            MODE_ADDITIVE,  // Actions from the scenario are applied
        } Mode;

        typedef struct
        {
            std::string      name;
            std::string      type;
            OSCProperties*   properties;
            Entities*        entities;
            ScenarioGateway* gateway;
            Parameters*      parameters;
        } InitArgs;

        enum class DomainActivation
        {
            UNDEFINED = 0,
            OFF       = 1,
            ON        = 2
        };

        Controller() : object_(0), entities_(0), gateway_(0), scenario_engine_(0), player_(0)
        {
        }
        Controller(InitArgs* args);
        virtual ~Controller() = default;

        static const char* GetTypeNameStatic()
        {
            return CONTROLLER_BASE_TYPE_NAME;
        }
        virtual const char* GetTypeName()
        {
            return GetTypeNameStatic();
        }
        static int GetTypeStatic()
        {
            return CONTROLLER_BASE_TYPE_ID;
        }
        virtual int GetType()
        {
            return GetTypeStatic();
        }

        virtual void Assign(Object* object);
        virtual void Activate(DomainActivation lateral, DomainActivation longitudinal)
        {
            int domain_mask = static_cast<int>(domain_);

            if (lateral == DomainActivation::OFF)
            {
                domain_mask &= ~static_cast<int>(ControlDomains::DOMAIN_LAT);
            }
            else if (lateral == DomainActivation::ON)
            {
                domain_mask |= static_cast<int>(ControlDomains::DOMAIN_LAT);
            }

            if (longitudinal == DomainActivation::OFF)
            {
                domain_mask &= ~static_cast<int>(ControlDomains::DOMAIN_LONG);
            }
            else if (longitudinal == DomainActivation::ON)
            {
                domain_mask |= static_cast<int>(ControlDomains::DOMAIN_LONG);
            }

            domain_ = static_cast<ControlDomains>(domain_mask);
        };
        virtual void Deactivate()
        {
            domain_ = ControlDomains::DOMAIN_NONE;
        };
        virtual void Init(){};
        virtual void ReportKeyEvent(int key, bool down);
        virtual void SetScenarioEngine(ScenarioEngine* scenario_engine)
        {
            scenario_engine_ = scenario_engine;
        };
        virtual void SetPlayer(ScenarioPlayer* player)
        {
            player_ = player;
        };

        // Base class Step function should be called from derived classes
        virtual void Step(double timeStep);

        bool Active()
        {
            return static_cast<int>(domain_) != 0;
        };
        std::string GetName()
        {
            return name_;
        }
        ControlDomains GetDomain()
        {
            return domain_;
        }
        int GetMode()
        {
            return mode_;
        }
        std::string Mode2Str(int mode);
        Object*     GetRoadObject()
        {
            return object_;
        }

        bool IsActiveOnDomains(ControlDomains domainMask);
        bool IsActiveOnAnyOfDomains(ControlDomains domainMask);
        bool IsActive();

    protected:
        ControlDomains   domain_;  // bitmask according to ControllerDomain type
        int              mode_;    // add to scenario actions or replace
        Object*          object_;  // The object to which the controller is attached and hence controls
        std::string      name_;
        std::string      type_name_;
        Entities*        entities_;
        ScenarioGateway* gateway_;
        ScenarioEngine*  scenario_engine_;
        ScenarioPlayer*  player_;
    };

    typedef Controller* (*ControllerInstantiateFunction)(void* args);
    Controller* InstantiateController(void* args);
}  // namespace scenarioengine