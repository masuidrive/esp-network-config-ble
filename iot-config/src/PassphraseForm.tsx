import React, { useState, useEffect } from "react";
import { makeStyles } from "@material-ui/core/styles";
import "./App.css";
import { SSIDItem } from "ble-smartconfig";
import Container from "@material-ui/core/Container";
import TextField from '@material-ui/core/TextField';

type Props = {
  ssidItem: SSIDItem
  onChange: (passphrase: string, validated: boolean) => void
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


function validate(passphrase: string) {
  return true;
}


export function PassphraseForm(props: Props) {
  const classes = useStyles();
  const [passphrase, setPassphrase] = useState<string>("");
  const [validated, setValidated] = useState<boolean>(false);

  return (
    <Container>
      <form className={classes.form} noValidate>
        <TextField
          variant="outlined"
          margin="normal"
          required
          fullWidth
          id="ssid"
          label="SSID"
          name="ssid"
          value={props.ssidItem.ssid}
          disabled={true}
          InputProps={{
            readOnly: true,
          }}
        />
        <TextField
          variant="outlined"
          margin="normal"
          required
          fullWidth
          name="passphrase"
          label="Passphrase"
          type="passphrase"
          id="password"
          autoFocus
          onChange={(event: any) => {
            const val = (event.target.value ?? "") as string;
            setValidated(validate(val));
            props.onChange(val, validated);
          }}
        />
      </form>
    </Container>
  );

}

