#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>

#include "MotorHatDriver.h"

#ifndef VERBOSE
  #define VERBOSE
#endif

/**
 * precompiler
**/
sdp_session_t *register_service( uint8_t rfcomm_channel );

int init_btserver( void       );
char *read_RFCOMM( int client );

/* To compile use:
*  use makefile
*/
int main( int argc, char *argv[] ){

   int sock;
   char *pStr;

   if( MotorDriverInit()  != 0 ){
      fprintf( stderr, "Stoping Application now due to error!\n" );
      exit( -1 );
   }

   if( (sock = init_btserver()) > 0 ){
      while( 1 ){
         int  i = 0, x = 0, y = 0;
         int  x_dir = 0;                /*x_dir == 0 move right, x_dir == 1 move left*/
         int  y_dir = FORWARD;         /*y_dir == -y move FORWARD, y_dir == y move BACKWARD*/
         char strX[4], strY[4];
         char *p = (char *) NULL;

         /*memset fills the str with 0 convenient for string manipulation*/
         memset( strX, 0, sizeof strX );
         memset( strY, 0, sizeof strY );

         /*use pStr values when received to control the robot */
         pStr = read_RFCOMM( sock );

         /*TODO refine*/
         p  = strstr( pStr, "x:" ); /*parse string for x value*/
         p += 2;                    /*move to digit value*/

         while( p[0] != ' ' ) {
            if( p[0] == '-' ) {     /*set direction on x axis: + -> right and - left*/
               x_dir = 1;           /* x_dir == 0 move right, x_dir == 1 move left*/
            } else {
               strX[i++] = p[0];
            }
            p++;
         }

         p  = strstr( pStr, "y:" ); /*parse string for y value*/
         p += 2;                    /*move to digit value*/
         i  = 0;                    /*reset for pointer pos.*/

         while( p[0] != '\0' ){     /*move until end of string*/
            if( p[0] == '-' ) {     /*set direction on y axis: +y -> BACKWARDS and -y FORWARD this is because y axis input is inverted*/
               y_dir = BACKWARD;
            } else {
               strY[i++] = p[0];
            }
            p++;
         }

#ifdef VERBOSE
   printf( "strX: %s, strY: %s, x_dir: %d, y_dir: %d\n", strX, strY, x_dir, y_dir );
#endif

         x = atoi( strX ); /*convert string x into integer*/
         y = atoi( strY ); /*convert string y into integer*/

         /*set all motor directions*/
         setDirectionalPin( m1, y_dir  );
         setDirectionalPin( m2, y_dir  );
         setDirectionalPin( m3, y_dir  );
         setDirectionalPin( m4, y_dir  );

         /*set speed of the motors since y is between 0 - 10 multiply by 25 to get nice values between 0 - 250*/
         if( x_dir ) setPWMSpeed( m1, y*25 - x*25 );
         else        setPWMSpeed( m1, y*25        );
         if( x_dir ) setPWMSpeed( m2, y*25        );
         else        setPWMSpeed( m2, y*25 - x*25 );
         setPWMSpeed( m3, y*25 );
         setPWMSpeed( m4, y*25 );

      }
   } else {
      perror( "Error: " );
   }

   return 0;
}

/*
 * Register a SDP service
 *
 * @param:  uint8_t rfcomm_cannel where to connect
 * @return: sdp_session_t data structure with service
**/
sdp_session_t *register_service( uint8_t rfcomm_channel ) {
   /*
   * This UUID is the Bluetooth Base UUID and is commonly used for simple Bluetooth applications.
   * The coresponding client/server needs the same UUID to connect to.
   *
   * UUID: 00001101-0000-1000-8000-00805F9B34FB
   */
   uint32_t svc_uuid_int[] = { 0x01110000, 0x00100000, 0x80000080, 0xFB349B5F };

   const char *service_name = "RobotController";
   const char *svc_dsc      = "Bluetooth Controller for RaspPI Robot";
   const char *service_prov = "RobotController Provider";

   int result, abort_flag = 0;

   uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid, svc_class_uuid;

   sdp_list_t *l2cap_list        = 0,
              *rfcomm_list       = 0,
              *root_list         = 0,
              *proto_list        = 0,
              *access_proto_list = 0,
              *svc_class_list    = 0,
              *profile_list      = 0;

   sdp_data_t         *channel = 0;
   sdp_profile_desc_t  profile;
   sdp_record_t        record  = { 0 };
   sdp_session_t      *session = 0;

   /*set service UUID*/
   sdp_uuid128_create( &svc_uuid, &svc_uuid_int );
   sdp_set_service_id( &record, svc_uuid        );

#ifdef VERBOSE
   char buf[256] = "";
        /*decode UUID*/
   sdp_uuid2strn( &svc_uuid, buf, 256 );
   printf( "Registering UUID %s\n", buf );
#endif

   /*set the service class*/
   sdp_uuid16_create( &svc_class_uuid, SERIAL_PORT_SVCLASS_ID );
   svc_class_list = sdp_list_append( 0, &svc_class_uuid );
   sdp_set_service_classes( &record, svc_class_list );

   /*set the bt profile information*/
   sdp_uuid16_create( &profile.uuid, SERIAL_PORT_PROFILE_ID );
   profile.version = 0x0100;
   profile_list    = sdp_list_append( 0, &profile );
   sdp_set_profile_descs( &record, profile_list   );

   /*make the service record publicly browsable*/
   sdp_uuid16_create( &root_uuid, PUBLIC_BROWSE_GROUP );
   root_list = sdp_list_append( 0, &root_uuid );
   sdp_set_browse_groups( &record, root_list  );

   /*set l2cap information*/
   sdp_uuid16_create( &l2cap_uuid, L2CAP_UUID   );
   l2cap_list = sdp_list_append( 0, &l2cap_uuid );
   proto_list = sdp_list_append( 0, l2cap_list  );

   /*register the RFCOMM channel for RFCOMM sockets*/
   sdp_uuid16_create( &rfcomm_uuid, RFCOMM_UUID );
   channel = sdp_data_alloc( SDP_UINT8, &rfcomm_channel );
   rfcomm_list = sdp_list_append( 0, &rfcomm_uuid );
   sdp_list_append( rfcomm_list, channel    );
   sdp_list_append( proto_list, rfcomm_list );

   access_proto_list = sdp_list_append( 0, proto_list );
   sdp_set_access_protos( &record, access_proto_list  );

   /*set the name, provider, and description*/
   sdp_set_info_attr( &record, service_name, service_prov, svc_dsc );

   /*connect to the local SDP server, register the service record, and disconnect*/
   session = sdp_connect( BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY );

   if( session == NULL ){
      printf( "Error: Session is null. Aborting! Try to run server with root rights.\n" );
      abort_flag = 1;
   } else {
      result = sdp_record_register( session, &record, 0 );
#ifdef DEBUG
   printf( "SDP record registered return: %d\n", result );
#endif
   }

   /*cleanup*/
   sdp_data_free( channel);
   sdp_list_free( l2cap_list,        0 );
   sdp_list_free( rfcomm_list,       0 );
   sdp_list_free( root_list,         0 );
   sdp_list_free( access_proto_list, 0 );
   sdp_list_free( svc_class_list,    0 );
   sdp_list_free( profile_list,      0 );

   if( abort_flag ){
       exit( EXIT_FAILURE );
   }

   return session;
}

/**
 * Start a bt server using RFCOMM with SDP
**/
int init_btserver( void ) {
   /* set port first available */
   int sock, client, result;
   uint8_t port = 1;

   struct sockaddr_rc loc_addr = { 0 },
                      rem_addr = { 0 };

   char buf[1024] = { 0 };

   socklen_t opt = sizeof( rem_addr );

   /*bluetooth adapter*/
   loc_addr.rc_family  =  AF_BLUETOOTH;
   loc_addr.rc_bdaddr  = *BDADDR_ANY;
   loc_addr.rc_channel = (uint8_t) port;

   /*register service*/
   sdp_session_t *session = register_service( port );

   /*allocate socket*/
   sock = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );

#ifdef VERBOSE
   printf( "RFCOMM socket bound to: %d\n", sock );
#endif

   /*bind socket to port*/
   result = bind( sock, (struct sockaddr *)&loc_addr, sizeof(loc_addr) );

#ifdef VERBOSE
   printf( "RFCOMM socket port bound to: %d on channel %d\n", port, result );
#endif

   /*put socket into listening mode*/
   result = listen( sock, 1 );

#ifdef VERBOSE
   printf( "RFCOMM socket listening: %d\n", result  );
   printf( "RFCOMM waiting for client accept ...\n" );
#endif

   /*accept a connection*/
   client = accept( sock, (struct sockaddr *)&rem_addr, &opt );

   ba2str( &rem_addr.rc_bdaddr, buf );
   fprintf( stdout, "RFCOMM accepted connection from %s\n", buf );
   memset(  buf, 0, sizeof(buf)     );

   return client;
}

/**
 * read data from RFCOMM Socket
**/
char *read_RFCOMM( int sock ) {
   char input[1024] = { 0 };
   int bytes_read;

   bytes_read = read( sock, input, sizeof( input ) );

   if ( bytes_read > 0 ) {
#ifdef VERBOSE
      printf( "RFCOMM value received: %s\n", input );
#endif
      return input;
   } else {
#ifdef VERBOSE
   printf( "Nothing received\n" );
#endif
      return "_rcv_no_input";
   }
}
