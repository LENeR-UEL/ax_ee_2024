import { useState } from "react"

export const useUpdate = () => {
  const [_, setValue] = useState(0);

  const trigger = () => {
    setValue(Date.now());
  };

  return trigger;
}
