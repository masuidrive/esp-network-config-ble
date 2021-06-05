import React, { useState, useEffect } from "react";
import { makeStyles } from "@material-ui/core/styles";
import "./App.css";
import { BLESmartConfig, SSIDItem } from "ble-smartconfig";
import Container from "@material-ui/core/Container";
import CircularProgress from '@material-ui/core/CircularProgress';

type Props = {
  smartConfig?: BLESmartConfig
  ssidItem: SSIDItem,
  passphrase?: string
  onComplete: (passohrase: string) => void
};

const useStyles = makeStyles((theme) => ({
  form: {
    width: '100%', // Fix IE 11 issue.
    marginTop: theme.spacing(1),
  },
  submit: {
    margin: theme.spacing(3, 0, 2),
  },
}));

enum Status {
  idle = 0,
  connecting,
  connected,
  failed,
}

export function WiFiConnect(props: Props) {
  const classes = useStyles();
  const [status, setStatus] = useState<Status>(Status.idle);

  useEffect(() => {
    if(props.ssidItem === undefined) {
      setStatus(Status.idle);
    }
    else {
      (async () => {
        setStatus(Status.connecting);
        if(!await props.smartConfig!.set_wifi(props.ssidItem.ssid, props.passphrase ?? "")) {
          setStatus(Status.failed);
          return;
        }

        const connected = await props.smartConfig!.test_wifi_connection();
        if(connected) {
          setStatus(Status.connected);
        }
        else {
          setStatus(Status.failed);
        }
      })();
    }
  }, [props.smartConfig, props.ssidItem, props.passphrase]);

  return (
    <Container>
      <div>Status: {status}</div>
    </Container>
  );

}

