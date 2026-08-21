/*
 * mod-city-bots — AiObjectContextAccess.h
 *
 * Access bypass for mod-playerbots private AiObjectContext shared registries.
 * See mod-dungeon-clear for the full rationale.
 */

#ifndef MOD_CITYBOTS_AIOBJECTCONTEXTACCESS_H
#define MOD_CITYBOTS_AIOBJECTCONTEXTACCESS_H

#include "Action.h"
#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "Strategy.h"
#include "Trigger.h"
#include "Value.h"

namespace cb_access
{
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

    template <typename Tag, typename Tag::type Ptr>
    struct Robber
    {
        friend typename Tag::type get(Tag) { return Ptr; }
    };

    struct StrategyListTag
    {
        typedef SharedNamedObjectContextList<Strategy>* type;
        friend type get(StrategyListTag);
    };
    struct ActionListTag
    {
        typedef SharedNamedObjectContextList<Action>* type;
        friend type get(ActionListTag);
    };
    struct TriggerListTag
    {
        typedef SharedNamedObjectContextList<Trigger>* type;
        friend type get(TriggerListTag);
    };
    struct ValueListTag
    {
        typedef SharedNamedObjectContextList<UntypedValue>* type;
        friend type get(ValueListTag);
    };

    template struct Robber<StrategyListTag, &AiObjectContext::sharedStrategyContexts>;
    template struct Robber<ActionListTag, &AiObjectContext::sharedActionContexts>;
    template struct Robber<TriggerListTag, &AiObjectContext::sharedTriggerContexts>;
    template struct Robber<ValueListTag, &AiObjectContext::sharedValueContexts>;

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    inline SharedNamedObjectContextList<Strategy>* SharedStrategyContexts()
    {
        return get(StrategyListTag{});
    }
    inline SharedNamedObjectContextList<Action>* SharedActionContexts()
    {
        return get(ActionListTag{});
    }
    inline SharedNamedObjectContextList<Trigger>* SharedTriggerContexts()
    {
        return get(TriggerListTag{});
    }
    inline SharedNamedObjectContextList<UntypedValue>* SharedValueContexts()
    {
        return get(ValueListTag{});
    }
}

#endif
