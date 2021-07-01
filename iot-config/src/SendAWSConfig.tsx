import React, { useState, useEffect } from "react";
import { makeStyles } from "@material-ui/core/styles";
import "./App.css";
import { BLESmartConfig, IoTConfig } from "ble-smartconfig";
import Container from "@material-ui/core/Container";

type Props = {
  smartConfig?: BLESmartConfig,
  iotConfig: IoTConfig,
  onComplete: (success: boolean) => void
};

enum Status {
  idle = 0,
  sending,
  sent,
  failed,
}

export function SendAWSConfig(props: Props) {
  const [status, setStatus] = useState<Status>(Status.idle);

  useEffect(() => {
    (async () => {
      setStatus(Status.sending);
      if(!await props.smartConfig!.set_awsiot(props.iotConfig)) {
        setStatus(Status.failed);
        props.onComplete(false);
        return;
      }
      props.onComplete(true);
      setStatus(Status.sent);
    })();
  }, [props.iotConfig,props.smartConfig]);

  return (
    <Container>
      <div>Settings...</div>
    </Container>
  );

}

