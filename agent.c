// $CC data.c -Ih/include/ -o data -lpthread -lwraclient -lrt
//  -Lsrc/client 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <wra.h>

int agent_send_string(wra_handle wra, char *data_item, char *data)
{
	wra_tm_handle tm_data;
	wra_status rc;

	if (!wra) {
		printf("ERROR: Invalid handle\n");
		return WRA_ERR_FAILED;
	}
	
	/* Allocate a telemetry data object */
	tm_data = wra_tm_create(WRA_TM_DATATM, data_item);
	if (tm_data == WRA_NULL)
	{
		printf("ERROR: Could not create an data telemetry object\n");
		return WRA_ERR_FAILED;
	}

	/* set telemetry data object attributes */
	rc = wra_tm_setvalue_int(tm_data, WRA_TM_ATTR_PRIORITY, WRA_TM_PRIO_HIGH);
	rc = wra_tm_setvalue_string(tm_data, WRA_TM_ATTR_DATA, data);

	/* send telemetry data object to the server */
	rc = wra_tm_post(wra, tm_data, WRA_NULL, WRA_NULL);
	if (rc == WRA_ERR_NO_MEMORY)
		printf("could not post tm %s - returned WRA_ERR_NO_MEMORY\n",
				"wra_tm_post");

	if (rc != WRA_SUCCESS)
		printf("could not post tm %s - returned %d\n", "wra_tm_post", rc);

	/* Deallocate telemetry data object */
	if ((rc = wra_tm_destroy(tm_data)) != WRA_SUCCESS)
		printf("failed to deallocate a telemetry data object\n");
	
	return WRA_SUCCESS;
}

wra_handle agent_init(void)
{
	wra_handle wra = wra_gethandle();
	if (wra == WRA_NULL)
		printf("ERROR: Could not get agent handle\n");
	
	return wra;
}

int agent_deinit(wra_handle wra)
{
	int rc;
	
	if (!wra)
		return WRA_ERR_BAD_PARAM;
	
	rc = wra_delete_handle(wra);
	
	if (rc != WRA_SUCCESS)
		printf("ERROR: Failed to delete the agent handle\n");
	
	return rc;
}
