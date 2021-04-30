/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "VehSize"
 * 	found in "asn/VehSize.asn"
 * 	`asn1c -fskeletons-copy -fnative-types -gen-PER -pdu=auto -no-gen-example -D ./src`
 */

#ifndef	_VehicleSize_H_
#define	_VehicleSize_H_


#include <asn_application.h>

/* Including external dependencies */
#include "VehicleWidth.h"
#include "VehicleLength.h"
#include "VehicleHeight.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* VehicleSize */
typedef struct VehicleSize {
	VehicleWidth_t	 width;
	VehicleLength_t	 length;
	VehicleHeight_t	*height	/* OPTIONAL */;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VehicleSize_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_VehicleSize;
extern asn_SEQUENCE_specifics_t asn_SPC_VehicleSize_specs_1;
extern asn_TYPE_member_t asn_MBR_VehicleSize_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _VehicleSize_H_ */
#include <asn_internal.h>
