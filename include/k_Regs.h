#ifndef __K_REGS_H
#define __K_REGS_H

#define K_REG_DEFINE(name, type, address) namespace k { namespace reg { volatile type* const name = (volatile type* const)(address); } }
#define K_REG_DEFINE_STRUCT(name, type, address) namespace k { namespace reg {type* const name = (type* const)(address); } }
#define K_REG_SET(name, value) (*k::reg::name) = value
#define K_REG_GET(name) (*k::reg::name)

#define K_REG_GET_ARRAY(type, address, step, index) ((volatile type* const)((address) + (step) * (index)))
#define K_REG_GET_ARRAY_NV(type, address, step, index) ((type* const)((address) + (step) * (index)))

#endif