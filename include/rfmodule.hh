
#include <Arduino.h>

class RFModule
{
    public:
        RFModule(uint8_t pin_busy, uint8_t pin_ptt, bool allow_ptt_on_busy = false);

        bool IsRxBusy();

        bool IsTx();

        bool IsBusy();

        void SetPtt(bool tx);

    private:
        uint8_t _pin_busy;
        uint8_t _pin_ptt;
        bool _allow_ptt_on_busy;
        bool _ptt_state;
};
