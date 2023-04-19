defmodule ExYyjson do
  # Load the NIF library
  @on_load :load_nif

  # Define the NIF functions
  def encode(value), do: :erlang.nif_error(:nif_not_loaded)
  def decode(string), do: :erlang.nif_error(:nif_not_loaded)

  def decode!(string) do
    case decode(string) do
      {:ok, value} -> value
      {:error, reason} -> raise ArgumentError, reason
    end
  end

  # Load the NIF library from the priv directory
  defp load_nif do
    target = Atom.to_string(Mix.target())
    path = Application.app_dir(:ex_yyjson, ["priv", target, "exyyjson"])
    IO.inspect(path)
    :erlang.load_nif(path, 0)
  end
end
