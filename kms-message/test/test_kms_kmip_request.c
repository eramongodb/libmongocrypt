#include "test_kms_assert.h"

#include "kms_message/kms_kmip_request.h"

/*
<RequestMessage tag="0x420078" type="Structure">
 <RequestHeader tag="0x420077" type="Structure">
  <ProtocolVersion tag="0x420069" type="Structure">
   <ProtocolVersionMajor tag="0x42006a" type="Integer" value="1"/>
   <ProtocolVersionMinor tag="0x42006b" type="Integer" value="0"/>
  </ProtocolVersion>
  <BatchCount tag="0x42000d" type="Integer" value="1"/>
 </RequestHeader>
 <BatchItem tag="0x42000f" type="Structure">
  <Operation tag="0x42005c" type="Enumeration" value="3"/>
  <RequestPayload tag="0x420079" type="Structure">
   <ObjectType tag="0x420057" type="Enumeration" value="7"/>
   <TemplateAttribute tag="0x420091" type="Structure">
    <Attribute tag="0x420008" type="Structure">
     <AttributeName tag="0x42000a" type="TextString" value="Cryptographic Usage
Mask"/> <AttributeValue tag="0x42000b" type="Integer" value="0"/>
    </Attribute>
   </TemplateAttribute>
   <SecretData tag="0x420085" type="Structure">
    <SecretDataType tag="0x420086" type="Enumeration" value="2"/>
    <KeyBlock tag="0x420040" type="Structure">
     <KeyFormatType tag="0x420042" type="Enumeration" value="2"/>
     <KeyValue tag="0x420045" type="Structure">
     <KeyMaterial tag="0x420043" type="ByteString"
value="ffa8cc79e8c3763b0121fcd06bb3488c8bf42c0774604640279b16b264194030eeb08396241defcc4d32d16ea831ad777138f08e2f985664c004c2485d6f4991eb3d9ec32802537836a9066b4e10aeb56a5ccf6aa46901e625e3400c7811d2ec"/>
     </KeyValue>
    </KeyBlock>
   </SecretData>
  </RequestPayload>
 </BatchItem>
</RequestMessage>
*/
#define REGISTER_SECRETDATA_REQUEST                                           \
   0x42, 0x00, 0x78, 0x01, 0x00, 0x00, 0x01, 0x50, 0x42, 0x00, 0x77, 0x01,    \
      0x00, 0x00, 0x00, 0x38, 0x42, 0x00, 0x69, 0x01, 0x00, 0x00, 0x00, 0x20, \
      0x42, 0x00, 0x6a, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, \
      0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x6b, 0x02, 0x00, 0x00, 0x00, 0x04, \
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x0d, 0x02, \
      0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, \
      0x42, 0x00, 0x0f, 0x01, 0x00, 0x00, 0x01, 0x08, 0x42, 0x00, 0x5c, 0x05, \
      0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, \
      0x42, 0x00, 0x79, 0x01, 0x00, 0x00, 0x00, 0xf0, 0x42, 0x00, 0x57, 0x05, \
      0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, \
      0x42, 0x00, 0x91, 0x01, 0x00, 0x00, 0x00, 0x38, 0x42, 0x00, 0x08, 0x01, \
      0x00, 0x00, 0x00, 0x30, 0x42, 0x00, 0x0a, 0x07, 0x00, 0x00, 0x00, 0x18, \
      0x43, 0x72, 0x79, 0x70, 0x74, 0x6f, 0x67, 0x72, 0x61, 0x70, 0x68, 0x69, \
      0x63, 0x20, 0x55, 0x73, 0x61, 0x67, 0x65, 0x20, 0x4d, 0x61, 0x73, 0x6b, \
      0x42, 0x00, 0x0b, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, \
      0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x85, 0x01, 0x00, 0x00, 0x00, 0x98, \
      0x42, 0x00, 0x86, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, \
      0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x40, 0x01, 0x00, 0x00, 0x00, 0x80, \
      0x42, 0x00, 0x42, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, \
      0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x45, 0x01, 0x00, 0x00, 0x00, 0x68, \
      0x42, 0x00, 0x43, 0x08, 0x00, 0x00, 0x00, 0x60, 0xff, 0xa8, 0xcc, 0x79, \
      0xe8, 0xc3, 0x76, 0x3b, 0x01, 0x21, 0xfc, 0xd0, 0x6b, 0xb3, 0x48, 0x8c, \
      0x8b, 0xf4, 0x2c, 0x07, 0x74, 0x60, 0x46, 0x40, 0x27, 0x9b, 0x16, 0xb2, \
      0x64, 0x19, 0x40, 0x30, 0xee, 0xb0, 0x83, 0x96, 0x24, 0x1d, 0xef, 0xcc, \
      0x4d, 0x32, 0xd1, 0x6e, 0xa8, 0x31, 0xad, 0x77, 0x71, 0x38, 0xf0, 0x8e, \
      0x2f, 0x98, 0x56, 0x64, 0xc0, 0x04, 0xc2, 0x48, 0x5d, 0x6f, 0x49, 0x91, \
      0xeb, 0x3d, 0x9e, 0xc3, 0x28, 0x02, 0x53, 0x78, 0x36, 0xa9, 0x06, 0x6b, \
      0x4e, 0x10, 0xae, 0xb5, 0x6a, 0x5c, 0xcf, 0x6a, 0xa4, 0x69, 0x01, 0xe6, \
      0x25, 0xe3, 0x40, 0x0c, 0x78, 0x11, 0xd2, 0xec

#define REGISTER_SECRETDATA_SECRETDATA                                        \
   0xff, 0xa8, 0xcc, 0x79, 0xe8, 0xc3, 0x76, 0x3b, 0x01, 0x21, 0xfc, 0xd0,    \
      0x6b, 0xb3, 0x48, 0x8c, 0x8b, 0xf4, 0x2c, 0x07, 0x74, 0x60, 0x46, 0x40, \
      0x27, 0x9b, 0x16, 0xb2, 0x64, 0x19, 0x40, 0x30, 0xee, 0xb0, 0x83, 0x96, \
      0x24, 0x1d, 0xef, 0xcc, 0x4d, 0x32, 0xd1, 0x6e, 0xa8, 0x31, 0xad, 0x77, \
      0x71, 0x38, 0xf0, 0x8e, 0x2f, 0x98, 0x56, 0x64, 0xc0, 0x04, 0xc2, 0x48, \
      0x5d, 0x6f, 0x49, 0x91, 0xeb, 0x3d, 0x9e, 0xc3, 0x28, 0x02, 0x53, 0x78, \
      0x36, 0xa9, 0x06, 0x6b, 0x4e, 0x10, 0xae, 0xb5, 0x6a, 0x5c, 0xcf, 0x6a, \
      0xa4, 0x69, 0x01, 0xe6, 0x25, 0xe3, 0x40, 0x0c, 0x78, 0x11, 0xd2, 0xec

void kms_kmip_request_register_secretdata_test (void); // -Wmissing-prototypes: for testing only.
void
kms_kmip_request_register_secretdata_test (void)
{
   kms_request_t *req;
   uint8_t secret_data[] = {REGISTER_SECRETDATA_SECRETDATA};
   const uint8_t *actual_bytes;
   size_t actual_len;
   uint8_t expected_bytes[] = {REGISTER_SECRETDATA_REQUEST};
   size_t expected_len = sizeof (expected_bytes);

   req = kms_kmip_request_register_secretdata_new (
      NULL, secret_data, sizeof (secret_data));
   ASSERT_REQUEST_OK (req);

   actual_bytes = kms_request_to_bytes (req, &actual_len);
   ASSERT (actual_bytes != NULL);

   ASSERT_CMPBYTES (expected_bytes, expected_len, actual_bytes, actual_len);

   kms_request_destroy (req);
}

void kms_kmip_request_register_secretdata_invalid_test (void); // -Wmissing-prototypes: for testing only.
void
kms_kmip_request_register_secretdata_invalid_test (void)
{
   kms_request_t *req;
   uint8_t secret_data[KMS_KMIP_REQUEST_SECRETDATA_LENGTH] = {0};

   req = kms_kmip_request_register_secretdata_new (
      NULL, secret_data, KMS_KMIP_REQUEST_SECRETDATA_LENGTH - 1);
   ASSERT_REQUEST_ERROR (req, "SecretData length");

   kms_request_destroy (req);
}


/*
<RequestMessage tag="0x420078" type="Structure">
 <RequestHeader tag="0x420077" type="Structure">
  <ProtocolVersion tag="0x420069" type="Structure">
   <ProtocolVersionMajor tag="0x42006a" type="Integer" value="1"/>
   <ProtocolVersionMinor tag="0x42006b" type="Integer" value="0"/>
  </ProtocolVersion>
  <BatchCount tag="0x42000d" type="Integer" value="1"/>
 </RequestHeader>
 <BatchItem tag="0x42000f" type="Structure">
  <Operation tag="0x42005c" type="Enumeration" value="10"/>
  <RequestPayload tag="0x420079" type="Structure">
   <UniqueIdentifier tag="0x420094" type="TextString"
value="7FJYvnV6XkaUCWuY96bCSc6AuhvkPpqI"/>
  </RequestPayload>
 </BatchItem>
</RequestMessage>
*/
#define GET_REQUEST                                                           \
   0x42, 0x00, 0x78, 0x01, 0x00, 0x00, 0x00, 0x88, 0x42, 0x00, 0x77, 0x01,    \
      0x00, 0x00, 0x00, 0x38, 0x42, 0x00, 0x69, 0x01, 0x00, 0x00, 0x00, 0x20, \
      0x42, 0x00, 0x6a, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, \
      0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x6b, 0x02, 0x00, 0x00, 0x00, 0x04, \
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x0d, 0x02, \
      0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, \
      0x42, 0x00, 0x0f, 0x01, 0x00, 0x00, 0x00, 0x40, 0x42, 0x00, 0x5c, 0x05, \
      0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, \
      0x42, 0x00, 0x79, 0x01, 0x00, 0x00, 0x00, 0x28, 0x42, 0x00, 0x94, 0x07, \
      0x00, 0x00, 0x00, 0x20, 0x37, 0x46, 0x4a, 0x59, 0x76, 0x6e, 0x56, 0x36, \
      0x58, 0x6b, 0x61, 0x55, 0x43, 0x57, 0x75, 0x59, 0x39, 0x36, 0x62, 0x43, \
      0x53, 0x63, 0x36, 0x41, 0x75, 0x68, 0x76, 0x6b, 0x50, 0x70, 0x71, 0x49

void kms_kmip_request_get_test (void); // -Wmissing-prototypes: for testing only.
void
kms_kmip_request_get_test (void)
{
   kms_request_t *req;

   const uint8_t *actual_bytes;
   size_t actual_len;
   uint8_t expected_bytes[] = {GET_REQUEST};
   size_t expected_len = sizeof (expected_bytes);
   static const char *const GET_UNIQUE_IDENTIFIER =
      "7FJYvnV6XkaUCWuY96bCSc6AuhvkPpqI";

   req = kms_kmip_request_get_new (NULL, GET_UNIQUE_IDENTIFIER);
   ASSERT_REQUEST_OK (req);

   actual_bytes = kms_request_to_bytes (req, &actual_len);

   ASSERT (actual_bytes != NULL);
   ASSERT_CMPBYTES (actual_bytes, actual_len, expected_bytes, expected_len);

   kms_request_destroy (req);
}


/*
<RequestMessage tag="0x420078" type="Structure">
 <RequestHeader tag="0x420077" type="Structure">
  <ProtocolVersion tag="0x420069" type="Structure">
   <ProtocolVersionMajor tag="0x42006a" type="Integer" value="1"/>
   <ProtocolVersionMinor tag="0x42006b" type="Integer" value="0"/>
  </ProtocolVersion>
  <BatchCount tag="0x42000d" type="Integer" value="1"/>
 </RequestHeader>
 <BatchItem tag="0x42000f" type="Structure">
  <Operation tag="0x42005c" type="Enumeration" value="18"/>
  <RequestPayload tag="0x420079" type="Structure">
   <UniqueIdentifier tag="0x420094" type="TextString"
value="7FJYvnV6XkaUCWuY96bCSc6AuhvkPpqI"/>
  </RequestPayload>
 </BatchItem>
</RequestMessage>
*/
#define ACTIVATE_REQUEST                                                      \
   0x42, 0x00, 0x78, 0x01, 0x00, 0x00, 0x00, 0x88, 0x42, 0x00, 0x77, 0x01,    \
      0x00, 0x00, 0x00, 0x38, 0x42, 0x00, 0x69, 0x01, 0x00, 0x00, 0x00, 0x20, \
      0x42, 0x00, 0x6a, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, \
      0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x6b, 0x02, 0x00, 0x00, 0x00, 0x04, \
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x0d, 0x02, \
      0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, \
      0x42, 0x00, 0x0f, 0x01, 0x00, 0x00, 0x00, 0x40, 0x42, 0x00, 0x5c, 0x05, \
      0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, \
      0x42, 0x00, 0x79, 0x01, 0x00, 0x00, 0x00, 0x28, 0x42, 0x00, 0x94, 0x07, \
      0x00, 0x00, 0x00, 0x20, 0x37, 0x46, 0x4a, 0x59, 0x76, 0x6e, 0x56, 0x36, \
      0x58, 0x6b, 0x61, 0x55, 0x43, 0x57, 0x75, 0x59, 0x39, 0x36, 0x62, 0x43, \
      0x53, 0x63, 0x36, 0x41, 0x75, 0x68, 0x76, 0x6b, 0x50, 0x70, 0x71, 0x49

void kms_kmip_request_activate_test (void); // -Wmissing-prototypes: for testing only.
void
kms_kmip_request_activate_test (void)
{
   kms_request_t *req;

   const uint8_t *actual_bytes;
   size_t actual_len;
   uint8_t expected_bytes[] = {ACTIVATE_REQUEST};
   size_t expected_len = sizeof (expected_bytes);
   static const char *const ACTIVATE_UNIQUE_IDENTIFIER =
      "7FJYvnV6XkaUCWuY96bCSc6AuhvkPpqI";

   req = kms_kmip_request_activate_new (NULL, ACTIVATE_UNIQUE_IDENTIFIER);
   ASSERT_REQUEST_OK (req);

   actual_bytes = kms_request_to_bytes (req, &actual_len);
   ASSERT (actual_bytes != NULL);
   ASSERT_CMPBYTES (actual_bytes, actual_len, expected_bytes, expected_len);

   kms_request_destroy (req);
}
