// #include "DjiMotor.hpp"

// namespace Can
// {
// namespace Dji
// {

// template <uint8_t N> void DjiMotorBase<N>::Parse(const CAN_RxHeaderTypeDef RxHeader, const uint8_t *pData)
// {
//     const uint16_t received_id = CAN_ID(RxHeader);

//     for (uint8_t i = 0; i < N; ++i)
//     {
//         if (received_id == init_address + idxs[i])
//         {
//             memcpy(&feedback_[i], pData, sizeof(DjiMotorFeedback));
//             feedback_[i].angle = __builtin_bswap16(feedback_[i].angle);
//             feedback_[i].velocity = __builtin_bswap16(feedback_[i].velocity);
//             feedback_[i].current = __builtin_bswap16(feedback_[i].current);
//             break;
//         }
//     }
// }

// template <uint8_t N> void DjiMotorBase<N>::setMSD(send_data *msd, int16_t data, int id)
// {
//     msd->Data[(id - 1) * 2] = data >> 8;
//     msd->Data[(id - 1) * 2 + 1] = data << 8 >> 8;
// }

// template <uint8_t N> DjiMotorBase<N>::DjiMotorBase(uint16_t can_id, const uint8_t (&ids)[N]) : init_address(can_id)
// {
//     for (uint8_t i = 0; i < N; ++i)
//     {
//         idxs[i] = ids[i];
//     }
// }

// template <uint8_t N> void DjiMotorBase<N>::_Motor_ID_IDX_BIND_(uint8_t *ids, uint8_t size)
// {
//     uint8_t idxs[_Motor_ID_IDX_BIND_SIZE_];

//     for (uint8_t i = 0; i < _Motor_ID_IDX_BIND_SIZE_; i++) // 标记
//     {
//         this->idxs[i] = 0xff;
//     }
//     for (uint8_t i = 0; i < size; i++) // 绑定
//     {
//         this->idxs[ids[i]] = i;
//     }
// }

// template <uint8_t N> int DjiMotorBase<N>::GET_Motor_ID_ADDRESS_BIND_(int address)
// {
//     int idx = address - (this->init_address);
//     if (idx < 0)
//         return -1;
//     if (idx >= _Motor_ID_IDX_BIND_SIZE_)
//         return -1;
//     if (this->idxs[idx] == 0xff)
//         return -1;

//     return this->idxs[idx];
// }

// template <uint8_t N> GM2006<N>::GM2006(uint16_t can_id, const uint8_t (&ids)[N]) : DjiMotorBase<N>(can_id, ids)
// {
// }

// template <uint8_t N> float GM2006<N>::getAngle(float n)
// {
//     return 0;
// }

// template <uint8_t N> float GM2006<N>::getLastAngle(float n)
// {
//     return 0;
// }

// template <uint8_t N> float GM2006<N>::getAddAngle(float n)
// {
//     return 0;
// }

// template <uint8_t N> float GM2006<N>::getCurrent(float n)
// {
//     return 0;
// }

// template <uint8_t N> float GM2006<N>::getTorque(float n)
// {
//     return 0;
// }

// template <uint8_t N> float GM2006<N>::getTemperature(float n)
// {
//     return 0;
// }

// // 显式实例化模板类
// template class DjiMotorBase<2>;
// template class GM2006<2>;

// } // namespace Dji

// } // namespace Can