#pragma once

namespace WPEFramework {
namespace ORBBrowser {
class StateAwareMemoryObserver : public Exchange::IMemory,
    public PluginHost::IStateControl::INotification {
    Exchange::IMemory *_memory;
    PluginHost::IStateControl::state _state;
    uint64_t _holdOffTime;

    const uint64_t DEFAULT_HOLDOFF_TIMEOUT_IN_MS = 3 * 1000;

    uint32_t Multiplier() const
    {
        if (_state != PluginHost::IStateControl::SUSPENDED)
            return 1;
        if (_holdOffTime > Core::Time::Now().Ticks())
            return 1;
        return 2;
    }

public:
    StateAwareMemoryObserver(Exchange::IMemory *memory)
        : _memory(memory)
        , _state(PluginHost::IStateControl::RESUMED)
        , _holdOffTime(0)
    {
        _memory->AddRef();
    }

    ~StateAwareMemoryObserver()
    {
        _memory->Release();
    }

    uint64_t Resident() const override
    {
        return Multiplier() * _memory->Resident();
    }

    uint64_t Allocated() const override
    {
        return _memory->Allocated();
    }

    uint64_t Shared() const override
    {
        return _memory->Shared();
    }

    uint8_t Processes() const override
    {
        return _memory->Processes();
    }

    const bool IsOperational() const override
    {
        return _memory->IsOperational();
    }

    void StateChange(const PluginHost::IStateControl::state state) override
    {
        _holdOffTime = Core::Time::Now().Add(DEFAULT_HOLDOFF_TIMEOUT_IN_MS).Ticks();
        std::atomic_thread_fence(std::memory_order_seq_cst);
        _state = state;
    }

    static Exchange::IMemory* Wrap(Exchange::IMemory *memory)
    {
        if (memory)
            return Core::Service<StateAwareMemoryObserver>::Create<Exchange::IMemory>(memory);
        return nullptr;
    }

    BEGIN_INTERFACE_MAP(StateAwareMemoryObserver)
    INTERFACE_ENTRY(Exchange::IMemory)
    INTERFACE_ENTRY(PluginHost::IStateControl::INotification)
    END_INTERFACE_MAP
};
}  // namespace ORBBrowser
}  // namespace WPEFramework
