
#include "rfmodule.hh"

RFModule::RFModule(uint8_t pin_busy, uint8_t pin_ptt, bool allow_ptt_on_busy) :
    _pin_busy(pin_busy), _pin_ptt(pin_ptt), _allow_ptt_on_busy(allow_ptt_on_busy)
{
    // Setup pins direction
    pinMode(_pin_busy, INPUT_PULLUP); // _pin_busy as INPUT
    pinMode(_pin_ptt, OUTPUT); // _pin_ptt as OUTPUT

    SetPtt(false);
}

bool RFModule::IsRxBusy()
{
    return digitalRead(_pin_busy);
}

bool RFModule::IsTx()
{
    return _ptt_state;
}

bool RFModule::IsBusy()
{
    return IsRxBusy() | IsTx();
}

void RFModule::SetPtt(bool tx)
{
    if (tx &&
        !_allow_ptt_on_busy &&
        IsRxBusy())
    {
        return;
    }

    digitalWrite(_pin_ptt, tx);
    _ptt_state = tx;
}
