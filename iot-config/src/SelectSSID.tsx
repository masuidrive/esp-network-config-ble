import React, { useState, useEffect } from "react";
import { makeStyles } from "@material-ui/core/styles";
import "./App.css";
import { BLESmartConfig, SSIDItem } from "ble-smartconfig";
import Container from "@material-ui/core/Container";
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import ListItemIcon from '@material-ui/core/ListItemIcon';
import ListItemText from '@material-ui/core/ListItemText';
import Divider from '@material-ui/core/Divider';
import LockIcon from '@material-ui/icons/Lock';
import EditIcon from '@material-ui/icons/Edit';
import CachedIcon from '@material-ui/icons/Cached';

type Props = {
  smartConfig?: BLESmartConfig
  onSelect: (ssidItem?: SSIDItem) => void
};

const useStyles = makeStyles((theme) => ({
}));

function uniqueSSIDItem(items: SSIDItem[]): SSIDItem[] {
  console.log(items);
  const result:SSIDItem[] = [];
  const map = new Set<string>();
  for (const item of items) {
      if(!map.has(item.ssid)){
          map.add(item.ssid);    // set any value to Map
          result.push(item);
      }
  }
  return result;
}

export function SelectSSID(props: Props) {
  const classes = useStyles();
  const [items, setItems] = useState<SSIDItem[] | undefined>(undefined);
  useEffect(() => {
    if(props.smartConfig !== undefined) {
      (async () => {
        setItems(uniqueSSIDItem(await props.smartConfig!.list_ssid()));
      })();
    }
  }, [props.smartConfig]);

  if(props.smartConfig === undefined) {
    return <></>;
  }

  if(items === undefined) {
    return <div>loading...</div>
  }
  else {
    return (
      <Container>
        <List>
          {items!.map((item) =>
            <ListItem button key={item.ssid} onClick={() => props.onSelect(item)}>
              <ListItemIcon>
              {
                item.isOpen() ? <></> :
                  <LockIcon />
              }
              </ListItemIcon>
              <ListItemText
                primary={item.ssid}
              />
            </ListItem>
          )}
          <ListItem button>
            <ListItemIcon>
              <EditIcon />
            </ListItemIcon>
            <ListItemText
              primary="Other..."
            />
          </ListItem>
          <Divider />
          <ListItem button>
            <ListItemIcon>
              <CachedIcon />
            </ListItemIcon>
            <ListItemText
              primary="Refresh the list"
            />
          </ListItem>
        </List>
      </Container>
    );
  }
}

